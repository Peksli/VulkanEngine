#include "VulkanAbstraction/Core/VulkanPhysicalDevice.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/LogSystem.h"


namespace VulkanEngine {

    static const std::vector<const char*> s_DeviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    VulkanPhysicalDevice::VulkanPhysicalDevice()
    {
        auto* ctx = VulkanContext::GetRaw();

        // Get all devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(*ctx->GetInstance(), &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(*ctx->GetInstance(), &deviceCount, devices.data());

        if (devices.empty())
            VulkanEngine_CRITICAL("No Vulkan physical devices found!");

        // Select the best one
        m_PhysicalDevice = SelectBestDevice(devices);

        if (m_PhysicalDevice == VK_NULL_HANDLE)
            VulkanEngine_CRITICAL("Failed to find a suitable GPU!");

        // Store indices
        m_Indices = FindQueueFamilies(m_PhysicalDevice);

        VulkanEngine_INFO(fmt::runtime("Selected GPU: {}"), GetName());
    }

    VkPhysicalDevice VulkanPhysicalDevice::SelectBestDevice(const std::vector<VkPhysicalDevice>& devices)
    {
        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
        uint32_t maxScore = 0;

        for (const auto& device : devices)
        {
            uint32_t score = RateDeviceSuitability(device);
            if (score > maxScore)
            {
                bestDevice = device;
                maxScore = score;
            }
        }

        return bestDevice;
    }

    uint32_t VulkanPhysicalDevice::RateDeviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        // Feature Chain setup 
        VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

        features2.pNext     = &features13;
        features13.pNext    = &features12;

        vkGetPhysicalDeviceFeatures2(device, &features2);

        // Check Hard Requirements
        bool extensionsSupported    = CheckExtensionSupport(device);
        bool queuesSupported        = FindQueueFamilies(device).IsComplete();
        bool featuresSupported      = 
            features13.dynamicRendering     && 
            features13.synchronization2     &&
            features12.bufferDeviceAddress  && 
            features12.descriptorIndexing;

        if (!extensionsSupported || !queuesSupported || !featuresSupported)
            return 0;

        // Calculate Score
        uint32_t score = 1;

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;

        score += props.limits.maxImageDimension2D;

        return score;
    }

    QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;
        auto* ctx = VulkanContext::GetRaw();

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (int i = 0; i < queueFamilies.size(); i++)
        {
            const auto& flags = queueFamilies[i].queueFlags;

            // Graphics
            if (flags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphics = i;

            // Presentation
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *ctx->GetSurface(), &presentSupport);
            if (presentSupport)
                indices.presentation = i;

            if (indices.IsComplete())
                break;
        }

        return indices;
    }

    bool VulkanPhysicalDevice::CheckExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());

        std::set<std::string> required(s_DeviceExtensions.begin(), s_DeviceExtensions.end());

        for (const auto& extension : availableExtensions)
            required.erase(extension.extensionName);

        return required.empty();
    }

    std::string VulkanPhysicalDevice::GetName() const
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

        return std::string(props.deviceName);
    }

}