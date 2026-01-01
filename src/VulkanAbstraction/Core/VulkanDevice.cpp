#include "VulkanAbstraction/Core/VulkanDevice.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

    VulkanDevice::VulkanDevice()
    {
        auto* ctx = VulkanContext::GetRaw();
        auto& physDevice = *ctx->GetPhysicalDevice();

        // Prepare Queue Create Infos
        // std::set to ensure we only create one queue per unique family index
        std::set<uint32_t> uniqueQueueFamilies =
        {
            physDevice.GetGraphicsFamily(),
            physDevice.GetTransferFamily(),
            physDevice.GetPresentationFamily()
        };

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;

        for (uint32_t familyIndex : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queueInfo.queueFamilyIndex  = familyIndex;
            queueInfo.queueCount        = 1;
            queueInfo.pQueuePriorities  = &queuePriority;
            queueCreateInfos.push_back(queueInfo);
        }

        // Enable Features
        VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        features12.bufferDeviceAddress  = VK_TRUE;
        features12.descriptorIndexing   = VK_TRUE;
        features12.pNext = nullptr;

        VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        features13.dynamicRendering = VK_TRUE;
        features13.synchronization2 = VK_TRUE;
        features13.pNext = &features12;

        VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        features2.pNext = &features13;

        // Define Extensions
        const std::vector<const char*> deviceExtensions = 
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        // Create Logical Device
        VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        createInfo.pNext = &features2;
        createInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos        = queueCreateInfos.data();
        createInfo.enabledExtensionCount    = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames  = deviceExtensions.data();

        CHECK_VK_RES(vkCreateDevice(physDevice.GetRaw(), &createInfo, nullptr, &m_Device));

        // Retrieve Queues
        vkGetDeviceQueue(m_Device, physDevice.GetGraphicsFamily(),      0,  &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, physDevice.GetTransferFamily(),      0,  &m_TransferQueue);
        vkGetDeviceQueue(m_Device, physDevice.GetPresentationFamily(),  0,  &m_PresentationQueue);
    }

    void VulkanDevice::Shutdown()
    {
        if (m_Device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_Device, nullptr);
            m_Device = VK_NULL_HANDLE;
        }
    }

}