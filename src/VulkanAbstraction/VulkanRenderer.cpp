#include "VulkanAbstraction/VulkanRenderer.h"
#include "Core/Application.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

    // ------------------------------------------------------------------------------------
    // FRAME
    // ------------------------------------------------------------------------------------

    // Creates command pool, cmd buffers, and sync primitives
    void Frame::Init(VkDevice device, uint32_t queueFamilyIndex)
    {
        VkCommandPoolCreateInfo poolInfo = VulkanUtils::GetCommandPoolInfo(
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            queueFamilyIndex);
        CHECK_VK_RES(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

        VkCommandBufferAllocateInfo allocInfo = VulkanUtils::GetCmdBufferAllocateInfo(commandPool, 1);
        CHECK_VK_RES(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

        VkSemaphoreCreateInfo semInfo = VulkanUtils::GetSemaphoreCreateInfo();
        VkFenceCreateInfo fenceInfo = VulkanUtils::GetFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        CHECK_VK_RES(vkCreateSemaphore(device, &semInfo, nullptr, &imageAvailableSemaphore));
        CHECK_VK_RES(vkCreateFence(device, &fenceInfo, nullptr, &renderFinishedFence));
    }

    // Clear frame data
    void Frame::Cleanup(VkDevice device)
    {
        if (commandPool) vkDestroyCommandPool(device, commandPool, nullptr);
        if (imageAvailableSemaphore) vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        if (renderFinishedFence) vkDestroyFence(device, renderFinishedFence, nullptr);
    }

    // ------------------------------------------------------------------------------------
    // VULKAN RENDERER
    // ------------------------------------------------------------------------------------

    // Creates render semaphores, init frames and image state
    VulkanRenderer::VulkanRenderer()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();
        size_t imageCount = Application::GetRaw()->GetSwapchain()->GetImages().size();

        InitSemaphores(device, imageCount);
        InitFrames();
        InitImageStates(imageCount);
    }

    void VulkanRenderer::InitSemaphores(VkDevice device, size_t count)
    {
        m_RenderFinishedSemaphores.reserve(count);
        VkSemaphoreCreateInfo info = VulkanUtils::GetSemaphoreCreateInfo();

        for (uint32_t i = 0; i < count; i++)
        {
            VkSemaphore sem{ VK_NULL_HANDLE };
            CHECK_VK_RES(vkCreateSemaphore(device, &info, nullptr, &sem));
            m_RenderFinishedSemaphores.push_back(sem);
        }
    }

    void VulkanRenderer::InitFrames()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();
        const auto& physDevice = ctx->GetPhysicalDevice();

        for (auto& frame : m_GraphicsFrames)
            frame.Init(device, physDevice->GetGraphicsFamily());
    }

    void VulkanRenderer::InitImageStates(size_t count)
    {
        m_ImagesState.resize(count);
        for (auto& state : m_ImagesState) 
        {
            state.currentStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            state.currentAccess = 0;
            state.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    void VulkanRenderer::Shutdown()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();
        vkDeviceWaitIdle(device);

        for (auto& frame : m_GraphicsFrames) frame.Cleanup(device);
        for (auto& sem : m_RenderFinishedSemaphores) vkDestroySemaphore(device, sem, nullptr);
        m_RenderFinishedSemaphores.clear();
    }

    void VulkanRenderer::OpenRenderScope()
    {
        auto* ctx = VulkanContext::GetRaw();
        auto* app = Application::GetRaw();
        VkDevice device = *ctx->GetDevice();
        Frame& currentFrame = m_GraphicsFrames[m_CurrentFrameIndex];

        // Wait till prev frame done
        CHECK_VK_RES(vkWaitForFences(device, 1, &currentFrame.renderFinishedFence, VK_TRUE, UINT64_MAX));

        // Acquire image from swapchain
        CHECK_VK_RES(vkAcquireNextImageKHR(
            device,
            app->GetSwapchain()->GetRaw(),
            UINT64_MAX,
            currentFrame.imageAvailableSemaphore,
            VK_NULL_HANDLE,
            &m_CurrentImageIndex));

        // Prepare for begin
        CHECK_VK_RES(vkResetFences(device, 1, &currentFrame.renderFinishedFence));
        CHECK_VK_RES(vkResetCommandBuffer(currentFrame.commandBuffer, 0));
        VkCommandBufferBeginInfo beginInfo = VulkanUtils::GetBeginCmdBufferInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        CHECK_VK_RES(vkBeginCommandBuffer(currentFrame.commandBuffer, &beginInfo));
    }

    void VulkanRenderer::Clear(glm::vec3 clearColor)
    {
        VkCommandBuffer cmd = m_GraphicsFrames[m_CurrentFrameIndex].commandBuffer;
        auto& imageInfo = Application::GetRaw()->GetSwapchain()->GetImages()[m_CurrentImageIndex];

        // Barrier: Prepare image for Rendering (Smth -> Attachment)
        VulkanUtils::InsertImageMemoryBarrier(cmd, imageInfo.Image,
            m_ImagesState[m_CurrentImageIndex],
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkClearColorValue clearValue;
        clearValue = { { clearColor.r, clearColor.g, clearColor.b, 1.0f } };

        VkImageSubresourceRange range = VulkanUtils::GetImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdClearColorImage(cmd, imageInfo.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &range);
    }

    void VulkanRenderer::EndRenderScope()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkCommandBuffer cmd = m_GraphicsFrames[m_CurrentFrameIndex].commandBuffer;
        VkImage image = Application::GetRaw()->GetSwapchain()->GetImages()[m_CurrentImageIndex].Image;

        // Barrier: Prepare image for Present (Smth -> Present)
        VulkanUtils::InsertImageMemoryBarrier(cmd, image,
            m_ImagesState[m_CurrentImageIndex],
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        CHECK_VK_RES(vkEndCommandBuffer(cmd));

        // Submit
        VkCommandBufferSubmitInfo cmdInfo = VulkanUtils::GetCommandBufferSubmitInfo(cmd);

        // Wait for image availability (from Acquire)
        VkSemaphoreSubmitInfo waitInfo = VulkanUtils::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            m_GraphicsFrames[m_CurrentFrameIndex].imageAvailableSemaphore);

        // Signal rendering finished (per swapchain image)
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

        vkQueuePresentKHR(ctx->GetDevice()->GetPresentationQueue(), &presentInfo);

        AdvanceFrame();
    }
}