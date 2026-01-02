#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>


namespace VulkanEngine {

    struct QueueFamilyIndices
    {
        int32_t graphics        = -1;
        int32_t presentation    = -1;

        bool IsComplete() const 
        {
            return graphics > -1 && presentation > -1;
        }
    };

    class VulkanPhysicalDevice
    {
    public:
        VulkanPhysicalDevice();
        virtual ~VulkanPhysicalDevice() = default;

        VkPhysicalDevice      GetRaw()  const { return m_PhysicalDevice; }
        std::string           GetName() const;

        uint32_t GetGraphicsFamily()     const { return static_cast<uint32_t>(m_Indices.graphics); }
        uint32_t GetPresentationFamily() const { return static_cast<uint32_t>(m_Indices.presentation); }

        operator VkPhysicalDevice() const { return m_PhysicalDevice; }

    private:
        VkPhysicalDevice     SelectBestDevice(const std::vector<VkPhysicalDevice>& devices);
        uint32_t             RateDeviceSuitability(VkPhysicalDevice device);
        QueueFamilyIndices   FindQueueFamilies(VkPhysicalDevice device);
        bool                 CheckExtensionSupport(VkPhysicalDevice device);

    private:
        VkPhysicalDevice    m_PhysicalDevice = VK_NULL_HANDLE;
        QueueFamilyIndices  m_Indices;
    };

}