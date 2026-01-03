#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <vector>
#include <glm/glm.hpp>

#include "VulkanAbstraction/VulkanTypes.h"

namespace VulkanEngine {

    static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

    class VulkanRenderer
    {
    public:
        VulkanRenderer();
        virtual ~VulkanRenderer() = default;

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        void OpenRenderScope();
        void EndRenderScope();
        void Clear(glm::vec3 clearColor);

    private:
        void InitFrames();
        void InitSemaphores(VkDevice device, size_t count);
        void InitPreRenderTarget();
        void SubmitAndPresent();

        void AdvanceFrame() { m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT; }

    private:
        std::array<Frame, FRAMES_IN_FLIGHT> m_GraphicsFrames;
        std::vector<VkSemaphore>    m_RenderFinishedSemaphores;

        AllocatedImage m_PreRenderTarget; // will be copied into swapchain image

        uint32_t m_CurrentFrameIndex = 0;
        uint32_t m_CurrentImageIndex = 0;
    };
}