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
        InitImageStates(imageCount);

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

        VulkanUtils::InsertImageMemoryBarrier(cmd, imageInfo.Image,
            m_ImagesState[m_CurrentImageIndex],
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,        
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

        VulkanUtils::InsertImageMemoryBarrier(cmd, image,
            m_ImagesState[m_CurrentImageIndex],
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0, 
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        CHECK_VK_RES(vkEndCommandBuffer(cmd));

        // SUBMIT
        VkCommandBufferSubmitInfo cmdInfo = VulkanUtils::GetCommandBufferSubmitInfo(cmd);

        VkSemaphoreSubmitInfo waitInfo = VulkanUtils::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            m_GraphicsFrames[m_CurrentFrameIndex].imageAvailableSemaphore);

        VkSemaphoreSubmitInfo signalInfo = VulkanUtils::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
            m_RenderFinishedSemaphores[m_CurrentImageIndex]); // Используем семафор, привязанный к картинке

        VkSubmitInfo2 submitInfo = VulkanUtils::GetSubmitInfo(&cmdInfo, &signalInfo, &waitInfo);

        CHECK_VK_RES(vkQueueSubmit2(ctx->GetDevice()->GetGraphicsQueue(), 1, &submitInfo, m_GraphicsFrames[m_CurrentFrameIndex].renderFinishedFence));

        // PRESENT
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
}