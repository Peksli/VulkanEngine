#include "VulkanAbstraction/VulkanSwapchain.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h" 
#include "Utility/Utility.h"

#include <GLFW/glfw3.h>


namespace VulkanEngine {

    VulkanSwapchain::VulkanSwapchain()
    {
        auto* ctx = VulkanContext::GetRaw();

        VkPhysicalDevice    physicalDevice  = *ctx->GetPhysicalDevice();
        VkDevice            device          = *ctx->GetDevice();
        VkSurfaceKHR        surface         = *ctx->GetSurface();

        // Query Details
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

        // Select Settings
        VkSurfaceFormatKHR surfaceFormat    = ChooseSurfaceFormat(formats);
        VkPresentModeKHR   presentMode      = ChoosePresentMode(presentModes);
        VkExtent2D         extent           = ChooseExtent(capabilities);

        // Save for future use
        m_Format = surfaceFormat.format;
        m_Extent = extent;

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

        // Create Swapchain
        VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        // Queue Families Handling
        uint32_t indices[] =
        {
            ctx->GetPhysicalDevice()->GetGraphicsFamily(),
            ctx->GetPhysicalDevice()->GetPresentationFamily()
        };

        if (indices[0] != indices[1])
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = indices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        CHECK_VK_RES(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_Swapchain));

        // Retrieve Images and Create Views
        InitImages();

        // Deletor
        auto* app = Application::GetRaw();
        for (const auto& img : m_Images)
        {
            if (img.imageView != VK_NULL_HANDLE)
                app->GetLifetimeManager()->Push(vkDestroyImageView, device, img.imageView, nullptr);
        }

        app->GetLifetimeManager()->Push(vkDestroySwapchainKHR, device, m_Swapchain, nullptr);
    }

    void VulkanSwapchain::InitImages()
    {
        auto* ctx = VulkanContext::GetRaw();
        VkDevice device = *ctx->GetDevice();

        // Get VkImage from Swapchain
        uint32_t count;
        vkGetSwapchainImagesKHR(device, m_Swapchain, &count, nullptr);
        std::vector<VkImage> rawImages(count);
        vkGetSwapchainImagesKHR(device, m_Swapchain, &count, rawImages.data());

        m_Images.resize(count);

        // Image view
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_Format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        for (size_t i = 0; i < count; ++i)
        {
            // Set VkImage
            m_Images[i].image = rawImages[i];

            // Set VkImageView
            viewInfo.image = rawImages[i];
            CHECK_VK_RES(vkCreateImageView(device, &viewInfo, nullptr, &m_Images[i].imageView));

            // Set Extent
            m_Images[i].imageExtent = { m_Extent.width, m_Extent.height, 1 };
        }
    }

    // ------------------------------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------------------------------

    VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const
    {
        for (const auto& fmt : formats)
        {
            if (fmt.format == VK_FORMAT_R8G8B8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return fmt;

            if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return fmt;
        }
        return formats[0];
    }

    VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes) const
    {
        for (const auto& mode : modes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
    {
        // if 1:1 map screen coords and pixels
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            auto* app = Application::GetRaw();
            auto* ctx = VulkanContext::GetRaw();
            int width, height;
            glfwGetFramebufferSize(
                app->GetWindow()->GetRaw(),
                &width, &height
            );

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
}