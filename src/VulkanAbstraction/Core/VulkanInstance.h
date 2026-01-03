#pragma once

#include <vulkan/vulkan.h>
#include <vector>


namespace VulkanEngine {

    class VulkanInstance
    {
    public:
        VulkanInstance();
        virtual ~VulkanInstance() = default;
        VulkanInstance(const VulkanInstance&)               = delete;
        VulkanInstance& operator=(const VulkanInstance&)    = delete;

        VkInstance GetRaw()     const { return m_Instance; }
        operator VkInstance()   const { return m_Instance; }

    private:
        VkInstance m_Instance{ VK_NULL_HANDLE };

        std::vector<const char*> GetRequiredExtensions()    const;
        std::vector<const char*> GetRequiredLayers()        const;
    };

}