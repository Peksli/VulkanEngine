#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanAbstraction/VulkanTypes.h"


namespace VulkanEngine {

    struct SwapchainImage 
    {
        ImageState  imageState;
        VkImage     image       = VK_NULL_HANDLE;
        VkImageView imageView   = VK_NULL_HANDLE;
        VkExtent3D  imageExtent = { 0,0,0 };
    };

    class VulkanSwapchain
    {
    public:
        VulkanSwapchain();
        virtual ~VulkanSwapchain() = default;
        VulkanSwapchain(const VulkanSwapchain&)             = delete;
        VulkanSwapchain& operator=(const VulkanSwapchain&)  = delete;

        VkSwapchainKHR  GetRaw()    const { return m_Swapchain; }
        VkFormat        GetFormat() const { return m_Format;    }
        VkExtent2D      GetExtent() const { return m_Extent;    }

        std::vector<SwapchainImage>& GetImages() { return m_Images; }

    private:
        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)  const;
        VkPresentModeKHR   ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes)        const;
        VkExtent2D         ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)           const;

        void InitImages();

    private:
        VkSwapchainKHR              m_Swapchain = VK_NULL_HANDLE;
        std::vector<SwapchainImage> m_Images;

        VkFormat   m_Format;
        VkExtent2D m_Extent;
    };

}