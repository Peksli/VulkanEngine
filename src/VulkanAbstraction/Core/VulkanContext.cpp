#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"


namespace VulkanEngine  {

    VulkanContext* VulkanContext::s_Instance = nullptr;

    VulkanContext::VulkanContext()
    {
        s_Instance = this;

        auto app = Application::GetRaw();
        const auto& window = app->GetWindow();
        auto glfwWindow = window->GetRaw();

        m_Instance          = std::make_unique<VulkanInstance>();
        m_Debugger          = std::make_unique<VulkanDebugger>();
        m_Surface           = std::make_unique<VulkanSurface>(glfwWindow);
        m_PhysicalDevice    = std::make_unique<VulkanPhysicalDevice>();
        m_Device            = std::make_unique<VulkanDevice>();
    }

    VulkanContext::~VulkanContext()
    {
        s_Instance = nullptr;
    }

}
