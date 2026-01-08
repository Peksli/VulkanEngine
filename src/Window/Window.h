#pragma once

#include <string>
#include <GLFW/glfw3.h>


namespace VulkanEngine {

    struct WindowSpecification
    {
        std::string Title       = "Vulkan Engine";
        uint32_t    Width       = 1280;
        uint32_t    Height      = 720;
        bool        Resizable   = true;
    };

    class Window
    {
    public:
        Window(const WindowSpecification& spec);
        virtual ~Window() = default;
        Window(const Window&)               = delete;
        Window& operator=(const Window&)    = delete;

        void OnUpdate();
        bool ShouldClose();
        void Shutdown();

        operator GLFWwindow* ()     const { return m_Window;        }
        GLFWwindow* GetRaw()        const { return m_Window;        }
        uint32_t    GetWidth()      const { return m_Spec.Width;    }
        uint32_t    GetHeight()     const { return m_Spec.Height;   }

    private:
        void InitializeGLFW();

    private:
        GLFWwindow* m_Window{ nullptr };
        WindowSpecification m_Spec;
    };

}