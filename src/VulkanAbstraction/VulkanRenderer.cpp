#include "VulkanAbstraction/VulkanRenderer.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"

namespace VulkanEngine {

    // ------------------------------------------------------------------------------------
    // FRAME
    // ------------------------------------------------------------------------------------

    void Frame::Init(VkDevice device, uint32_t queueFamilyIndex)
    {
        auto* app = Application::GetRaw();

        // Command Pool
        VkCommandPoolCreateInfo poolInfo = VulkanUtils::GetCommandPoolInfo(
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            queueFamilyIndex);
        CHECK_VK_RES(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

        app->GetLifetimeManager()->Push(vkDestroyCommandPool, device, commandPool, nullptr);

        // Command Buffer
        VkCommandBufferAllocateInfo allocInfo = VulkanUtils::GetCmdBufferAllocateInfo(commandPool, 1);
        CHECK_VK_RES(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

        // Sync Objects
        VkSemaphoreCreateInfo semInfo = VulkanUtils::GetSemaphoreCreateInfo();
        VkFenceCreateInfo fenceInfo = VulkanUtils::GetFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        CHECK_VK_RES(vkCreateSemaphore(device, &semInfo, nullptr, &imageAvailableSemaphore));
        CHECK_VK_RES(vkCreateFence(device, &fenceInfo, nullptr, &renderFinishedFence));

        app->GetLifetimeManager()->Push(vkDestroySemaphore, device, imageAvailableSemaphore, nullptr);
        app->GetLifetimeManager()->Push(vkDestroyFence, device, renderFinishedFence, nullptr);
    }

    // ------------------------------------------------------------------------------------
    // VULKAN RENDERER
    // ------------------------------------------------------------------------------------

    VulkanRenderer::VulkanRenderer()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();
        size_t imageCount = Application::GetRaw()->GetSwapchain()->GetImages().size();

        InitSemaphores(device, imageCount);
        InitFrames();
        InitPreRenderTarget();

        // Deletor -> wait till all work done
        auto* app = Application::GetRaw();
        app->GetLifetimeManager()->Push(vkDeviceWaitIdle, device);
    
        VulkanEngine_INFO("Vulkan Renderer initialized");
    }

    void VulkanRenderer::InitSemaphores(VkDevice device, size_t count)
    {
        m_RenderFinishedSemaphores.reserve(count);
        VkSemaphoreCreateInfo info = VulkanUtils::GetSemaphoreCreateInfo();
        auto* app = Application::GetRaw();

        for (size_t i = 0; i < count; i++)
        {
            VkSemaphore sem{ VK_NULL_HANDLE };
            CHECK_VK_RES(vkCreateSemaphore(device, &info, nullptr, &sem));

            m_RenderFinishedSemaphores.push_back(sem);

            app->GetLifetimeManager()->Push(vkDestroySemaphore, device, sem, nullptr);
        }
    }

    void VulkanRenderer::InitFrames()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();
        uint32_t graphicsQueueIndex = ctx->GetPhysicalDevice()->GetGraphicsFamily();

        for (auto& frame : m_GraphicsFrames)
        {
            frame.Init(device, graphicsQueueIndex);
        }
    }

    void VulkanRenderer::InitPreRenderTarget()
    {
        auto* app = Application::GetRaw();
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();

        const auto& allocator = app->GetAllocator();

        // PreRenderTarget creation
        // Extent according to swapchain image
        VkExtent3D extent = app->GetSwapchain()->GetImages()[0].imageExtent;
        m_PreRenderTarget.extent = extent;

        // Usage flags of image
        VkImageUsageFlags usage{};
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        usage |= VK_IMAGE_USAGE_STORAGE_BIT; // image can be used to create a VkImageView suitable for occupying a VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 

        // Image creation on VRAM
        VkFormat hdrFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        VkImageCreateInfo imageInfo = VulkanUtils::GetImageCreateInfo(hdrFormat, extent, usage);
        m_PreRenderTarget.format = hdrFormat;

        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT; // dedicated mem block
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; // VRAM allocation, cz no map flag set
        allocator->AllocateImage(imageInfo, allocationInfo, &m_PreRenderTarget.image, &m_PreRenderTarget.allocation);

        // Image view creation
        VkImageViewCreateInfo imageViewInfo = VulkanUtils::GetImageViewCreateInfo(m_PreRenderTarget.image, hdrFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        CHECK_VK_RES(vkCreateImageView(device, &imageViewInfo, nullptr, &m_PreRenderTarget.imageView));

        // Deletor
        app->GetLifetimeManager()->Push(vkDestroyImageView, device, m_PreRenderTarget.imageView, nullptr);
        app->GetLifetimeManager()->Push(vmaDestroyImage, allocator->GetRaw(), m_PreRenderTarget.image, m_PreRenderTarget.allocation);
    }

    void VulkanRenderer::SubmitAndPresent()
    {
        auto* ctx = VulkanContext::GetRaw();
        auto* app = Application::GetRaw();

        VkCommandBuffer cmd = m_GraphicsFrames[m_CurrentFrameIndex].commandBuffer;

        // Submit
        VkCommandBufferSubmitInfo cmdInfo = VulkanUtils::GetCommandBufferSubmitInfo(cmd);

        VkSemaphoreSubmitInfo waitInfo = VulkanUtils::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            m_GraphicsFrames[m_CurrentFrameIndex].imageAvailableSemaphore);

        VkSemaphoreSubmitInfo signalInfo = VulkanUtils::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
            m_RenderFinishedSemaphores[m_CurrentImageIndex]);

        VkSubmitInfo2 submitInfo = VulkanUtils::GetSubmitInfo(&cmdInfo, &signalInfo, &waitInfo);

        CHECK_VK_RES(vkQueueSubmit2(ctx->GetDevice()->GetGraphicsQueue(), 1, &submitInfo, m_GraphicsFrames[m_CurrentFrameIndex].renderFinishedFence));

        // Present
        VkSwapchainKHR swapchain = Application::GetRaw()->GetSwapchain()->GetRaw();

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.pSwapchains = &swapchain;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentImageIndex];
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &m_CurrentImageIndex;

        CHECK_VK_RES(vkQueuePresentKHR(ctx->GetDevice()->GetPresentationQueue(), &presentInfo));

        AdvanceFrame();
    }

    void VulkanRenderer::OpenRenderScope()
    {
        auto* ctx = VulkanContext::GetRaw();
        auto* app = Application::GetRaw();
        VkDevice device = *ctx->GetDevice();

        Frame& currentFrame = m_GraphicsFrames[m_CurrentFrameIndex];

        CHECK_VK_RES(vkWaitForFences(device, 1, &currentFrame.renderFinishedFence, VK_TRUE, UINT64_MAX));

        CHECK_VK_RES(vkAcquireNextImageKHR(
            device,
            app->GetSwapchain()->GetRaw(),
            UINT64_MAX,
            currentFrame.imageAvailableSemaphore, 
            VK_NULL_HANDLE,
            &m_CurrentImageIndex));

        CHECK_VK_RES(vkResetFences(device, 1, &currentFrame.renderFinishedFence));
        CHECK_VK_RES(vkResetCommandBuffer(currentFrame.commandBuffer, 0));

        VkCommandBufferBeginInfo beginInfo = VulkanUtils::GetBeginCmdBufferInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        CHECK_VK_RES(vkBeginCommandBuffer(currentFrame.commandBuffer, &beginInfo));
    }

    void VulkanRenderer::Clear(glm::vec3 clearColor)
    {
        VkCommandBuffer cmd = m_GraphicsFrames[m_CurrentFrameIndex].commandBuffer;
        auto& imageInfo = Application::GetRaw()->GetSwapchain()->GetImages()[m_CurrentImageIndex];

        VulkanUtils::InsertImageMemoryBarrier(cmd, m_PreRenderTarget.image,
            m_PreRenderTarget.imageState,
            VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);        

        VkClearColorValue clearValue;
        clearValue = { { clearColor.r, clearColor.g, clearColor.b, 1.0f } };

        VkImageSubresourceRange range = VulkanUtils::GetImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdClearColorImage(cmd, m_PreRenderTarget.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &range);
    }

    void VulkanRenderer::EndRenderScope()
    {
        auto* ctx = VulkanContext::GetRaw();
        auto* app = Application::GetRaw();

        VkCommandBuffer cmd             = m_GraphicsFrames[m_CurrentFrameIndex].commandBuffer;
        SwapchainImage  swapchainImage  = app->GetSwapchain()->GetImages()[m_CurrentImageIndex];

        // Transfer pre render target to trasnfer src
        VulkanUtils::InsertImageMemoryBarrier(cmd, m_PreRenderTarget.image,
            m_PreRenderTarget.imageState,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        // Transfer swapchain image to transfer dst
        VulkanUtils::InsertImageMemoryBarrier(cmd, swapchainImage.image,
            swapchainImage.imageState,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Blit
        VulkanUtils::CopyImageToImage(cmd, m_PreRenderTarget.image, swapchainImage.image, m_PreRenderTarget.extent, swapchainImage.imageExtent);

        // Transfer swapchain image to present
        VulkanUtils::InsertImageMemoryBarrier(cmd, swapchainImage.image,
            swapchainImage.imageState,
            VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        CHECK_VK_RES(vkEndCommandBuffer(cmd));

        // Submit and present
        SubmitAndPresent();
    }
}