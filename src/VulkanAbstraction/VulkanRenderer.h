#pragma once

#include <vulkan/vulkan.h>
#include <array>


namespace VulkanEngine {

    // higher fps, higher input latency
    static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

    struct Frame
    {
        VkCommandPool commandPool{ VK_NULL_HANDLE };
        VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    };

    class VulkanRenderer
    {
    public:
        VulkanRenderer();
        virtual ~VulkanRenderer() = default;
        VulkanRenderer(const VulkanRenderer&)               = delete;
        VulkanRenderer& operator=(const VulkanRenderer&)    = delete;

        void InitFrames();
        void CleanupFrames(); 
        void Shutdown();

    private:
        void InitFrameArray(std::array<Frame, FRAMES_IN_FLIGHT>& frames, uint32_t queueFamilyIndex);

    private:
        std::array<Frame, FRAMES_IN_FLIGHT> m_GraphicsFrames;
        std::array<Frame, FRAMES_IN_FLIGHT> m_TransferFrames;
    };

}