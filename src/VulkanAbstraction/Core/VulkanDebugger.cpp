#include "VulkanAbstraction/Core/VulkanDebugger.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugger::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        switch (severity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            VulkanEngine_TRACE(fmt::runtime("[Diagnostic] {}"), pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            VulkanEngine_INFO(fmt::runtime("[Info] {}"), pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            VulkanEngine_WARN(fmt::runtime("[Warning] {}"), pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            VulkanEngine_CRITICAL(fmt::runtime("[Error] {}"), pCallbackData->pMessage);
            break;
        default:
            break;
        }
        return VK_FALSE;
    }

    VulkanDebugger::VulkanDebugger()
    {
		auto* ctx = VulkanContext::GetRaw();
		VkInstance instance = *ctx->GetInstance();

        if (instance == VK_NULL_HANDLE)
        {
            VulkanEngine_CRITICAL("Cannot create Debugger: Invalid Vulkan Instance");
            return;
        }

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr)
        {
            VkDebugUtilsMessengerCreateInfoEXT createInfo = GetDebugCreateInfo();
            CHECK_VK_RES(func(instance, &createInfo, nullptr, &m_Messenger));
            VulkanEngine_INFO("Vulkan Debugger attached successfully");
        }
        else
        {
            VulkanEngine_WARN("Debug Utils extension not available. Debugger not created.");
        }

        // Deletor
        if (m_Messenger != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                auto* app = Application::GetRaw();
                app->GetLifetimeManager()->Push(func, instance, m_Messenger, nullptr);
            }
        }
    }

	VkDebugUtilsMessengerCreateInfoEXT VulkanDebugger::GetDebugCreateInfo()
	{
		return VkDebugUtilsMessengerCreateInfoEXT{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.pNext = nullptr,
			.flags = 0,
			.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = DebugCallback,
			.pUserData = nullptr,
		};
	}

}