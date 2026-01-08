#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


namespace VulkanEngine {

    class VulkanSurface
    {
    public:
        VulkanSurface(GLFWwindow* window);
        virtual ~VulkanSurface() = default;
        VulkanSurface(const VulkanSurface&)             = delete;
        VulkanSurface& operator=(const VulkanSurface&)  = delete;

        VkSurfaceKHR GetRaw()   const { return m_Surface; }
        operator VkSurfaceKHR() const { return m_Surface; }

    private:
        VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };
    };

}