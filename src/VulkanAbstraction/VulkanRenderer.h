#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <vector>
#include <glm/glm.hpp>


namespace VulkanEngine {

    static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

    struct Frame
    {
        VkCommandPool   commandPool{ VK_NULL_HANDLE };
        VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
        VkFence         renderFinishedFence{ VK_NULL_HANDLE };
        VkSemaphore     imageAvailableSemaphore{ VK_NULL_HANDLE };

        void Init(VkDevice device, uint32_t queueFamilyIndex);
        void Cleanup(VkDevice device);
    };

    struct ImageState
    {
        VkPipelineStageFlags2 currentStage  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        VkAccessFlags2        currentAccess = 0;
        VkImageLayout         currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    class VulkanRenderer
    {
    public:
        VulkanRenderer();
        virtual ~VulkanRenderer() = default;
        VulkanRenderer(const VulkanRenderer&)               = delete;
        VulkanRenderer& operator=(const VulkanRenderer&)    = delete;

        void OpenRenderScope();
        void EndRenderScope();
        void Clear(glm::vec3 clearColor);
        void Shutdown();

    private:
        void InitFrames();
        void InitSemaphores(VkDevice device, size_t count);
        void InitImageStates(size_t count);
        void AdvanceFrame() { m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT; }

    private:
        std::array<Frame, FRAMES_IN_FLIGHT> m_GraphicsFrames;
        std::vector<VkSemaphore>    m_RenderFinishedSemaphores;
        std::vector<ImageState>     m_ImagesState; // per swapchain image

        uint32_t m_CurrentFrameIndex = 0;
        uint32_t m_CurrentImageIndex = 0; 
    };
}