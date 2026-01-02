#include "VulkanAbstraction/VulkanRenderer.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

    VulkanRenderer::VulkanRenderer()
    {
        InitFrames();
    }

    void VulkanRenderer::InitFrames()
    {
        auto* ctx = VulkanContext::GetRaw();
        const auto& physDevice = ctx->GetPhysicalDevice();

        // Graphics frames
        InitFrameArray(m_GraphicsFrames, physDevice->GetGraphicsFamily());

        // Transfer frames
        InitFrameArray(m_TransferFrames, physDevice->GetTransferFamily());
    }

    void VulkanRenderer::InitFrameArray(std::array<Frame, FRAMES_IN_FLIGHT>& frames, uint32_t queueFamilyIndex)
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();

        // Command pool info
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.pNext = nullptr;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        // Command buffer allocate info
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            // Create Command Pool
            CHECK_VK_RES(vkCreateCommandPool(device, &poolInfo, nullptr, &frames[i].commandPool));

            // Allocate Command Buffer
            allocInfo.commandPool = frames[i].commandPool;
            CHECK_VK_RES(vkAllocateCommandBuffers(device, &allocInfo, &frames[i].commandBuffer));
        }
    }

    void VulkanRenderer::CleanupFrames()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();

		// Lambda to cleanup a frame array
        auto cleanupArray = [&](std::array<Frame, FRAMES_IN_FLIGHT>& frames) 
            {
            for (auto& frame : frames)
            {
                if (frame.commandPool != VK_NULL_HANDLE) 
                {
                    vkDestroyCommandPool(device, frame.commandPool, nullptr);
                    frame.commandPool = VK_NULL_HANDLE;
                }
            }
            };

        cleanupArray(m_GraphicsFrames);
        cleanupArray(m_TransferFrames);
    }

    void VulkanRenderer::Shutdown()
    {
        CleanupFrames();
    }
}