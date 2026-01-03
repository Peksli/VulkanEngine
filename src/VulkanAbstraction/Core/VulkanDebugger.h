#pragma once

#include <vulkan/vulkan.h>


namespace VulkanEngine {

    class VulkanDebugger
    {
    public:
        VulkanDebugger();
        virtual ~VulkanDebugger() = default;
        VulkanDebugger(const VulkanDebugger&)               = delete;
        VulkanDebugger& operator=(const VulkanDebugger&)    = delete;

        static VkDebugUtilsMessengerCreateInfoEXT GetDebugCreateInfo();

        VkDebugUtilsMessengerEXT GetRaw()   const { return m_Messenger; }
        operator VkDebugUtilsMessengerEXT() const { return m_Messenger; }

    private:
        VkDebugUtilsMessengerEXT m_Messenger{ VK_NULL_HANDLE };

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
            VkDebugUtilsMessageTypeFlagsEXT             type,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*                                       pUserData);
    };

}