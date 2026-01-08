#include "VulkanRenderer.h"
#include "VulkanAbstraction/Descriptors/VulkanDescriptorSet.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"

namespace VulkanEngine {

	void VulkanRenderer::Init()
	{
		InitCore();
		InitFrameData();
		InitSyncObjects();
	}

	void VulkanRenderer::InitCore()
	{
		s_Context			= std::make_unique<VulkanContext>();
		s_Allocator			= std::make_unique<VulkanMemoryAllocator>();
		s_ImGuiRenderer		= std::make_unique<ImGuiRenderer>();

		InitRenderTarget();
	}

	void VulkanRenderer::InitRenderTarget()
	{
		auto* app = Application::GetRaw();
		auto* ctx = VulkanContext::GetRaw();
		VkDevice device = *ctx->GetDevice();

		// Match swapchain dimensions
		auto		swapchainExtent = ctx->GetSwaphain()->GetExtent();
		VkExtent3D	swapchainExtent3D = { swapchainExtent.width, swapchainExtent.height, 1 };
		VkFormat	format = VK_FORMAT_R16G16B16A16_SFLOAT; // HDR

		s_RenderTarget.format = format;
		s_RenderTarget.extent = swapchainExtent3D;

		VkImageUsageFlags usage =
			VK_IMAGE_USAGE_STORAGE_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		// Image creation
		VkImageCreateInfo imageInfo = VulkanUtils::GetImageCreateInfo(format, swapchainExtent3D, usage);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		s_Allocator->AllocateImage(imageInfo, allocInfo, &s_RenderTarget.image, &s_RenderTarget.allocation);

		// Image view
		VkImageViewCreateInfo viewInfo = VulkanUtils::GetImageViewCreateInfo(s_RenderTarget.image, format, VK_IMAGE_ASPECT_COLOR_BIT);
		vkCreateImageView(device, &viewInfo, nullptr, &s_RenderTarget.imageView);

		// Lifetime management
		app->GetLifetimeManager()->Push(vkDestroyImageView, device, s_RenderTarget.imageView, nullptr);
		app->GetLifetimeManager()->Push(vmaDestroyImage, s_Allocator->GetRaw(), s_RenderTarget.image, s_RenderTarget.allocation);
	}

	void VulkanRenderer::InitFrameData()
	{
		auto* ctx = VulkanContext::GetRaw();
		VkDevice  device = *ctx->GetDevice();
		uint32_t  queueFamily = s_Context->GetPhysicalDevice()->GetGraphicsFamily();

		for (auto& frame : s_Frames)
		{
			frame.Init(device, queueFamily);
		}
	}

	void VulkanRenderer::InitSyncObjects()
	{
		auto* app = Application::GetRaw();
		auto* ctx = VulkanContext::GetRaw();
		VkDevice	device = *ctx->GetDevice();

		size_t imageCount = ctx->GetSwaphain()->GetImages().size();
		s_RenderFinishedSemaphores.resize(imageCount);

		VkSemaphoreCreateInfo semaphoreInfo = VulkanUtils::GetSemaphoreCreateInfo();

		for (size_t i = 0; i < imageCount; ++i)
		{
			CHECK_VK_RES(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &s_RenderFinishedSemaphores[i]));

			app->GetLifetimeManager()->Push(vkDestroySemaphore, device, s_RenderFinishedSemaphores[i], nullptr);
		}

	}

	// ===========================================================================
	// Frame Control
	// ===========================================================================

	void VulkanRenderer::BeginFrame()
	{
		auto* ctx = VulkanContext::GetRaw();
		Frame& frame = s_Frames[s_CurrentFrameIndex];

		// Wait for GPU to finish previous frame
		CHECK_VK_RES(vkWaitForFences(*s_Context->GetDevice(), 1, &frame.renderFinishedFence, VK_TRUE, UINT64_MAX));

		// Acquire next swapchain image
		CHECK_VK_RES(vkAcquireNextImageKHR(
			*s_Context->GetDevice(), ctx->GetSwaphain()->GetRaw(), UINT64_MAX,
			frame.imageAvailableSemaphore, VK_NULL_HANDLE, &s_CurrentImageIndex
		));

		// Reset frame resources
		CHECK_VK_RES(vkResetFences(*s_Context->GetDevice(), 1, &frame.renderFinishedFence));
		CHECK_VK_RES(vkResetCommandBuffer(frame.commandBuffer, 0));

		// Begin command buffer recording
		VkCommandBufferBeginInfo beginInfo = VulkanUtils::GetBeginCmdBufferInfo();
		CHECK_VK_RES(vkBeginCommandBuffer(frame.commandBuffer, &beginInfo));
	}

	void VulkanRenderer::EndFrame()
	{
		VkCommandBuffer cmd = s_Frames[s_CurrentFrameIndex].commandBuffer;
		auto* ctx = VulkanContext::GetRaw();
		auto& swapchainImages = ctx->GetSwaphain()->GetImages();
		SwapchainImage& targetImage = swapchainImages[s_CurrentImageIndex];

		BlitSceneToSwapchain(cmd);

		// Prepare swapchain image for present
		VulkanUtils::InsertImageMemoryBarrier(
			cmd, targetImage.image, targetImage.imageState,
			VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);

		CHECK_VK_RES(vkEndCommandBuffer(cmd));

		// Submit & Present
		Frame& frame = s_Frames[s_CurrentFrameIndex];
		VkCommandBufferSubmitInfo cmdSubmitInfo = VulkanUtils::GetCommandBufferSubmitInfo(frame.commandBuffer);
		VkSemaphoreSubmitInfo waitSemaphoreInfo = VulkanUtils::GetSemaphoreSubmitInfo(
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, frame.imageAvailableSemaphore);
		VkSemaphoreSubmitInfo signalSemaphoreInfo = VulkanUtils::GetSemaphoreSubmitInfo(
			VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, s_RenderFinishedSemaphores[s_CurrentImageIndex]);

		VkSubmitInfo2 submitInfo = VulkanUtils::GetSubmitInfo(&cmdSubmitInfo, &signalSemaphoreInfo, &waitSemaphoreInfo);
		CHECK_VK_RES(vkQueueSubmit2(s_Context->GetDevice()->GetGraphicsQueue(), 1, &submitInfo, frame.renderFinishedFence));

		VkSwapchainKHR swapchain = ctx->GetSwaphain()->GetRaw();
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &s_CurrentImageIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &s_RenderFinishedSemaphores[s_CurrentImageIndex];

		CHECK_VK_RES(vkQueuePresentKHR(s_Context->GetDevice()->GetPresentationQueue(), &presentInfo));

		AdvanceFrame();
	}

	void VulkanRenderer::BeginImGui()
	{
		VkCommandBuffer cmd = s_Frames[s_CurrentFrameIndex].commandBuffer;

		s_ImGuiRenderer->BeginImGuiFrame();
	}

	void VulkanRenderer::EndImGui()
	{
		VkCommandBuffer cmd = s_Frames[s_CurrentFrameIndex].commandBuffer;
		auto* ctx = VulkanContext::GetRaw();
		auto& swapchainImages = ctx->GetSwaphain()->GetImages();
		SwapchainImage& targetImage = swapchainImages[s_CurrentImageIndex];

		s_ImGuiRenderer->EndImGuiFrame(cmd, s_RenderTarget);
	}

	void VulkanRenderer::AdvanceFrame()
	{
		s_CurrentFrameIndex = (s_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::BlitSceneToSwapchain(VkCommandBuffer cmd)
	{
		auto* ctx = VulkanContext::GetRaw();
		auto& swapchainImages = ctx->GetSwaphain()->GetImages();
		SwapchainImage& targetImage = swapchainImages[s_CurrentImageIndex];

		// Prepare RenderTarget for Transfer Read
		VulkanUtils::InsertImageMemoryBarrier(
			cmd, s_RenderTarget.image, s_RenderTarget.imageState,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		);

		// Prepare Swapchain Image for Transfer Write
		VulkanUtils::InsertImageMemoryBarrier(
			cmd, targetImage.image, targetImage.imageState,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		// Blit
		VulkanUtils::CopyImageToImage(
			cmd, s_RenderTarget.image, targetImage.image,
			s_RenderTarget.extent, targetImage.imageExtent
		);
	}

	// ===========================================================================
	// Render Commands
	// ===========================================================================

	void VulkanRenderer::Clear(const glm::vec3& clearColor)
	{
		VkCommandBuffer cmd = s_Frames[s_CurrentFrameIndex].commandBuffer;

		// Clear requires transfer dst
		VulkanUtils::InsertImageMemoryBarrier(
			cmd, s_RenderTarget.image, s_RenderTarget.imageState,
			VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		// Clear color image
		VkClearColorValue clearValue{ { clearColor.r, clearColor.g, clearColor.b, 1.0f } };
		VkImageSubresourceRange range = VulkanUtils::GetImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdClearColorImage(cmd, s_RenderTarget.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &range);
	}

	void VulkanRenderer::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint)
	{
		vkCmdBindPipeline(s_Frames[s_CurrentFrameIndex].commandBuffer, bindPoint, pipeline);
	}

	void VulkanRenderer::BindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, VkDescriptorSet set)
	{
		vkCmdBindDescriptorSets(
			s_Frames[s_CurrentFrameIndex].commandBuffer,
			bindPoint,
			layout,
			0,
			1,
			&set,
			0,
			nullptr
		);
	}

	void VulkanRenderer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		VkCommandBuffer cmd = s_Frames[s_CurrentFrameIndex].commandBuffer;

		// Transition to general layout for compute shader storage access
		VulkanUtils::InsertImageMemoryBarrier(
			cmd, s_RenderTarget.image, s_RenderTarget.imageState,
			VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT,
			VK_IMAGE_LAYOUT_GENERAL
		);

		vkCmdDispatch(cmd, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanRenderer::EndInit()
	{
		auto* app = Application::GetRaw();
		auto* ctx = VulkanContext::GetRaw();
		VkDevice	device = *ctx->GetDevice();

		app->GetLifetimeManager()->Push(vkDeviceWaitIdle, device);
	}

}