#pragma once

#include <memory>

#include "VulkanAbstraction/Core/VulkanInstance.h"
#include "VulkanAbstraction/Core/VulkanSurface.h"
#include "VulkanAbstraction/Core/VulkanPhysicalDevice.h"
#include "VulkanAbstraction/Core/VulkanDevice.h"
#include "VulkanAbstraction/Core/VulkanDebugger.h"
#include "VulkanAbstraction/VulkanSwapchain.h"


namespace VulkanEngine {

    class VulkanContext
    {
    public:
        VulkanContext();
        virtual ~VulkanContext();

        static VulkanContext* GetRaw() noexcept { return s_Instance; }

        const std::unique_ptr<VulkanDebugger>&          GetDebugger()       const noexcept { return m_Debugger;       }
        const std::unique_ptr<VulkanInstance>&          GetInstance()       const noexcept { return m_Instance;       }
        const std::unique_ptr<VulkanSurface>&           GetSurface()        const noexcept { return m_Surface;        }
        const std::unique_ptr<VulkanPhysicalDevice>&    GetPhysicalDevice() const noexcept { return m_PhysicalDevice; }
        const std::unique_ptr<VulkanDevice>&            GetDevice()         const noexcept { return m_Device;         }
        const std::unique_ptr<VulkanSwapchain>&         GetSwaphain()       const noexcept { return m_Swapchain;      }

    private:
        std::unique_ptr<VulkanDebugger>         m_Debugger;
        std::unique_ptr<VulkanInstance>         m_Instance;
        std::unique_ptr<VulkanSurface>          m_Surface;
        std::unique_ptr<VulkanPhysicalDevice>   m_PhysicalDevice;
        std::unique_ptr<VulkanDevice>           m_Device;
        std::unique_ptr<VulkanSwapchain>        m_Swapchain;

        static VulkanContext* s_Instance;
    };

}
