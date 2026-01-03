#include "Window.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

    static void GLFWErrorCallback(int error, const char* description)
    {
        VulkanEngine_ERROR(fmt::runtime("GLFW Error ({0}): {1}"), error, description);
    }

    Window::Window(const WindowSpecification& spec)
        : m_Spec(spec)
    {
        InitializeGLFW();
    }

    void Window::InitializeGLFW()
    {
        VulkanEngine_INFO(fmt::runtime("Creating Window: {0} ({1}x{2})"), m_Spec.Title, m_Spec.Width, m_Spec.Height);

        if (!glfwInit())
        {
            VulkanEngine_CRITICAL("Failed to initialize GLFW");
            return;
        }

        glfwSetErrorCallback(GLFWErrorCallback);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, m_Spec.Resizable ? GLFW_TRUE : GLFW_FALSE);

        m_Window = glfwCreateWindow(
            static_cast<int>(m_Spec.Width),
            static_cast<int>(m_Spec.Height),
            m_Spec.Title.c_str(),
            nullptr,
            nullptr
        );

        if (!m_Window)
        {
            VulkanEngine_CRITICAL("Failed to create GLFW window");
            glfwTerminate();
            return;
        }

        glfwSetWindowUserPointer(m_Window, &m_Spec);

        // Deletor
        auto* app = Application::GetRaw();
        if (m_Window)
        {
            app->GetLifetimeManager()->PushFunction(glfwTerminate);
            app->GetLifetimeManager()->Push(glfwDestroyWindow, m_Window);
        }
        else
        {
            VulkanEngine_ERROR("Failed to destroy glfw and window cz it's nullptr");
        }
    }

    void Window::OnUpdate()
    {
        glfwPollEvents();
    }

    bool Window::ShouldClose()
    {
		return glfwWindowShouldClose(m_Window);
    }

}