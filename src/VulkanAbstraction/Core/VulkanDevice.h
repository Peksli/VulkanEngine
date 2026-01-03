#pragma once

#include <vulkan/vulkan.h>


namespace VulkanEngine {

    class VulkanDevice
    {
    public:
        VulkanDevice();
        virtual ~VulkanDevice() = default;

        VkDevice GetRaw()                const { return m_Device;               }
        VkQueue  GetGraphicsQueue()      const { return m_GraphicsQueue;        }
        VkQueue  GetPresentationQueue()  const { return m_PresentationQueue;    }

        operator VkDevice() const { return m_Device; }

    private:
        VkDevice m_Device               = VK_NULL_HANDLE;
        VkQueue  m_GraphicsQueue        = VK_NULL_HANDLE;
        VkQueue  m_PresentationQueue    = VK_NULL_HANDLE;
    };

}