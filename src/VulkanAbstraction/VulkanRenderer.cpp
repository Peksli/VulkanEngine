#include "VulkanAbstraction/VulkanRenderer.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"

namespace VulkanEngine {

	// ================================================================================
	// Frame Management - Handles per-frame synchronization and command buffers
	// ================================================================================

	void Frame::Init(VkDevice device, uint32_t queueFamilyIndex)
	{
		auto* app = Application::GetRaw();

		// Create transient command pool for efficient frame-local operations
		VkCommandPoolCreateInfo poolInfo = VulkanUtils::GetCommandPoolInfo(
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			queueFamilyIndex);
		CHECK_VK_RES(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
		app->GetLifetimeManager()->Push(vkDestroyCommandPool, device, commandPool, nullptr);

		// Allocate single command buffer per frame
		VkCommandBufferAllocateInfo allocInfo = VulkanUtils::GetCmdBufferAllocateInfo(commandPool, 1);
		CHECK_VK_RES(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

		// Create synchronization primitives (semaphore + signaled fence)
		VkSemaphoreCreateInfo semInfo = VulkanUtils::GetSemaphoreCreateInfo();
		VkFenceCreateInfo fenceInfo = VulkanUtils::GetFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

		CHECK_VK_RES(vkCreateSemaphore(device, &semInfo, nullptr, &imageAvailableSemaphore));
		CHECK_VK_RES(vkCreateFence(device, &fenceInfo, nullptr, &renderFinishedFence));

		app->GetLifetimeManager()->Push(vkDestroySemaphore, device, imageAvailableSemaphore, nullptr);
		app->GetLifetimeManager()->Push(vkDestroyFence, device, renderFinishedFence, nullptr);
	}

	// ================================================================================
	// VulkanRenderer - Main rendering 
	// ================================================================================

	VulkanRenderer::VulkanRenderer()
	{
		auto*		app		= Application::GetRaw();
		auto*		ctx		= VulkanContext::GetRaw();
		VkDevice	device	= *ctx->GetDevice();
		size_t		swapchainImageCount = Application::GetRaw()->GetSwapchain()->GetImages().size();

		// Initialize 
		InitFrames();
		InitSemaphores(device, swapchainImageCount);
		InitPreRenderTarget();
		InitDescriptors();

		// Ensure all works done
		app->GetLifetimeManager()->Push(vkDeviceWaitIdle, device);

		VulkanEngine_INFO("VulkanRenderer initialized successfully");
	}

	void VulkanRenderer::InitSemaphores(VkDevice device, size_t count)
	{
		auto* app = Application::GetRaw();
		m_RenderFinishedSemaphores.reserve(count);

		VkSemaphoreCreateInfo semaphoreInfo = VulkanUtils::GetSemaphoreCreateInfo();

		for (size_t i = 0; i < count; ++i)
		{
			VkSemaphore semaphore;
			CHECK_VK_RES(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore));
			m_RenderFinishedSemaphores.push_back(semaphore);

			app->GetLifetimeManager()->Push(vkDestroySemaphore, device, semaphore, nullptr);
		}
	}

	void VulkanRenderer::InitFrames()
	{
		auto* ctx = VulkanContext::GetRaw();
		uint32_t graphicsQueueFamily = ctx->GetPhysicalDevice()->GetGraphicsFamily();

		for (auto& frame : m_GraphicsFrames)
		{
			frame.Init(*ctx->GetDevice(), graphicsQueueFamily);
		}
	}

	void VulkanRenderer::InitPreRenderTarget()
	{
		auto*		app			= Application::GetRaw();
		auto*		ctx			= VulkanContext::GetRaw();
		const auto& allocator	= app->GetAllocator();
		VkDevice	device		= *ctx->GetDevice();

		// Match swapchain extent for 1:1 pixel correspondence
		const auto& swapchainImages  = app->GetSwapchain()->GetImages();
		m_PreRenderTarget.extent	 = swapchainImages[0].imageExtent;

		// HDR format with storage support for compute shaders
		VkFormat hdrFormat		= VK_FORMAT_R16G16B16A16_SFLOAT;
		VkImageUsageFlags usage =
			VK_IMAGE_USAGE_STORAGE_BIT			|
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT		|
			VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		// Create HDR render target in VRAM
		VkImageCreateInfo imageInfo  = VulkanUtils::GetImageCreateInfo(hdrFormat, m_PreRenderTarget.extent, usage);
		m_PreRenderTarget.format	 = hdrFormat;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		allocator->AllocateImage(imageInfo, allocInfo, &m_PreRenderTarget.image, &m_PreRenderTarget.allocation);

		// Create compatible image view
		VkImageViewCreateInfo viewInfo = VulkanUtils::GetImageViewCreateInfo(m_PreRenderTarget.image, hdrFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		CHECK_VK_RES(vkCreateImageView(device, &viewInfo, nullptr, &m_PreRenderTarget.imageView));

		// Register for cleanup
		app->GetLifetimeManager()->Push(vkDestroyImageView, device, m_PreRenderTarget.imageView, nullptr);
		app->GetLifetimeManager()->Push(vmaDestroyImage, allocator->GetRaw(), m_PreRenderTarget.image, m_PreRenderTarget.allocation);
	}

	void VulkanRenderer::InitDescriptors()
	{
		auto* ctx = VulkanContext::GetRaw();
		auto* app = Application::GetRaw();
		VkDevice device = *ctx->GetDevice();

		// Descriptor allocator for storage image
		std::vector<PoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
		};
		m_SetAllocator = std::make_shared<VulkanDescriptorSetAllocator>(1, poolSizes);

		// Descriptor set layout for single storage image binding
		VulkanDescriptorSetLayoutBuilder builder;
		m_SetLayout = builder
			.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
			.Build();

		// Allocate and populate descriptor set
		m_Set = m_SetAllocator->Allocate(m_SetLayout);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout	= VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView		= m_PreRenderTarget.imageView;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstBinding		= 0;
		descriptorWrite.dstSet			= m_Set;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorWrite.pImageInfo		= &imageInfo;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

		// Register layout for cleanup
		app->GetLifetimeManager()->Push(vkDestroyDescriptorSetLayout, device, m_SetLayout, nullptr);
	}

	// ================================================================================
	// Begin/End frame rendering scope
	// ================================================================================

	void VulkanRenderer::OpenRenderScope()
	{
		Frame& currentFrame = m_GraphicsFrames[m_CurrentFrameIndex];
		auto* ctx = VulkanContext::GetRaw();
		auto* app = Application::GetRaw();
		VkDevice device = *ctx->GetDevice();

		// Wait for frame completion and acquire swapchain image
		CHECK_VK_RES(vkWaitForFences(device, 1, &currentFrame.renderFinishedFence, VK_TRUE, UINT64_MAX));

		CHECK_VK_RES(vkAcquireNextImageKHR(
			device, app->GetSwapchain()->GetRaw(), UINT64_MAX,
			currentFrame.imageAvailableSemaphore, VK_NULL_HANDLE, &m_CurrentImageIndex));

		// Reset for new frame
		CHECK_VK_RES(vkResetFences(device, 1, &currentFrame.renderFinishedFence));
		CHECK_VK_RES(vkResetCommandBuffer(currentFrame.commandBuffer, 0));

		// Begin recording
		VkCommandBufferBeginInfo beginInfo = VulkanUtils::GetBeginCmdBufferInfo(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		CHECK_VK_RES(vkBeginCommandBuffer(currentFrame.commandBuffer, &beginInfo));
	}

	void VulkanRenderer::Clear(glm::vec3 clearColor)
	{
		VkCommandBuffer cmd = m_GraphicsFrames[m_CurrentFrameIndex].commandBuffer;

		// Transition HDR target to clear layout
		VulkanUtils::InsertImageMemoryBarrier(cmd, m_PreRenderTarget.image,
			m_PreRenderTarget.imageState,
			VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Clear entire image
		VkClearColorValue clearValue{};
		clearValue.float32[0] = clearColor.r;
		clearValue.float32[1] = clearColor.g;
		clearValue.float32[2] = clearColor.b;
		clearValue.float32[3] = 1.0f;

		VkImageSubresourceRange range = VulkanUtils::GetImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
		vkCmdClearColorImage(cmd, m_PreRenderTarget.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &range);
	}

	void VulkanRenderer::EndRenderScope()
	{
		VkCommandBuffer cmd = m_GraphicsFrames[m_CurrentFrameIndex].commandBuffer;
		auto& swapchainImages = Application::GetRaw()->GetSwapchain()->GetImages();
		SwapchainImage& swapchainTarget = swapchainImages[m_CurrentImageIndex];

		// Prepare HDR target for blit (source)
		VulkanUtils::InsertImageMemoryBarrier(cmd, m_PreRenderTarget.image,
			m_PreRenderTarget.imageState,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// Prepare swapchain image for blit (destination)  
		VulkanUtils::InsertImageMemoryBarrier(cmd, swapchainTarget.image,
			swapchainTarget.imageState,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Blit HDR -> Swapchain (handles format conversion)
		VulkanUtils::CopyImageToImage(cmd, m_PreRenderTarget.image, swapchainTarget.image,
			m_PreRenderTarget.extent, swapchainTarget.imageExtent);

		// Prepare swapchain image for presentation
		VulkanUtils::InsertImageMemoryBarrier(cmd, swapchainTarget.image,
			swapchainTarget.imageState,
			VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		CHECK_VK_RES(vkEndCommandBuffer(cmd));
		SubmitAndPresent();
	}

	// ================================================================================
	// Submission Pipeline - Queue operations and presentation
	// ================================================================================

	void VulkanRenderer::SubmitAndPresent()
	{
		auto* ctx = VulkanContext::GetRaw();
		Frame& currentFrame = m_GraphicsFrames[m_CurrentFrameIndex];

		// Submit command buffer with synchronization
		VkCommandBufferSubmitInfo cmdSubmitInfo = VulkanUtils::GetCommandBufferSubmitInfo(currentFrame.commandBuffer);

		VkSemaphoreSubmitInfo waitSemaphore = VulkanUtils::GetSemaphoreSubmitInfo(
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			currentFrame.imageAvailableSemaphore);

		VkSemaphoreSubmitInfo signalSemaphore = VulkanUtils::GetSemaphoreSubmitInfo(
			VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
			m_RenderFinishedSemaphores[m_CurrentImageIndex]);

		VkSubmitInfo2 submitInfo = VulkanUtils::GetSubmitInfo(&cmdSubmitInfo, &signalSemaphore, &waitSemaphore);
		CHECK_VK_RES(vkQueueSubmit2(ctx->GetDevice()->GetGraphicsQueue(), 1, &submitInfo, currentFrame.renderFinishedFence));

		// Present swapchain image
		VkSwapchainKHR swapchain = Application::GetRaw()->GetSwapchain()->GetRaw();
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentImageIndex];
		presentInfo.pImageIndices = &m_CurrentImageIndex;

		CHECK_VK_RES(vkQueuePresentKHR(ctx->GetDevice()->GetPresentationQueue(), &presentInfo));

		// Advance to next frame
		AdvanceFrame();
	}

}
