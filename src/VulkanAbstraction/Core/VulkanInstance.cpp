#include "VulkanAbstraction/Core/VulkanInstance.h"
#include "VulkanAbstraction/Core/VulkanDebugger.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"

#include <GLFW/glfw3.h>


namespace VulkanEngine {

	VulkanInstance::VulkanInstance()
	{
		// App Info
		VkApplicationInfo appInfo = {
			.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext				= nullptr,
			.pApplicationName	= "VulkanEngine",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName		= "VulkanEngine",
			.engineVersion		= VK_MAKE_VERSION(1, 0, 0),
			.apiVersion			= VK_API_VERSION_1_4,
		};

		// Extensions and layers
		auto extensions = GetRequiredExtensions();
		auto layers		= GetRequiredLayers();

		// Debugger Info 
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = VulkanDebugger::GetDebugCreateInfo();

		// Instance Create Info
		VkInstanceCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo,
			.flags = 0,
			.pApplicationInfo		 = &appInfo,
			.enabledLayerCount		 = static_cast<uint32_t>(layers.size()),
			.ppEnabledLayerNames	 = layers.data(),
			.enabledExtensionCount	 = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data()
		};

		CHECK_VK_RES(vkCreateInstance(&createInfo, nullptr, &m_Instance));

		VulkanEngine_INFO("Vulkan Instance created successfully");

		// Deletor
		auto* app = Application::GetRaw();
		app->GetLifetimeManager()->Push(vkDestroyInstance, m_Instance, nullptr);
	}


    std::vector<const char*> VulkanInstance::GetRequiredExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        if (!glfwExtensions)
        {
            VulkanEngine_CRITICAL("Failed to find GLFW extensions!");
            return {};
        }

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    std::vector<const char*> VulkanInstance::GetRequiredLayers() const
    {
        std::vector<const char*> layers;
        layers.push_back("VK_LAYER_KHRONOS_validation");

        return layers;
    }

}