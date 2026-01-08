#include "VulkanAbstraction/Core/VulkanSurface.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

    VulkanSurface::VulkanSurface(GLFWwindow* window)
    {
        auto* ctx = VulkanContext::GetRaw();
        VkInstance instance = *ctx->GetInstance();

        // Validation
        if (instance == VK_NULL_HANDLE)
        {
            VulkanEngine_CRITICAL("Cannot create Surface: Invalid Vulkan Instance");
            return;
        }

        if (!window)
        {
            VulkanEngine_CRITICAL("Cannot create Surface: Window handle is null");
            return;
        }

        // Create
        CHECK_VK_RES(glfwCreateWindowSurface(instance, window, nullptr, &m_Surface));
        VulkanEngine_INFO("Vulkan Surface created");

        // Deletor
        if (m_Surface != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
        {
            auto* app = Application::GetRaw();
            app->GetLifetimeManager()->Push(vkDestroySurfaceKHR, instance, m_Surface, nullptr);
        }
    }

}