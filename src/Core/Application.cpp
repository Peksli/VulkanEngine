#include "Core/Application.h"
#include "Core/LogSystem.h"


namespace VulkanEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
	{
		s_Instance = this;

		// Log system
		LogSystem::Initialize();

		// GLFW window
		WindowSpecification winSpec;
		winSpec.Width	= spec.width;
		winSpec.Height	= spec.height;
		winSpec.Title	= spec.windowName;
		winSpec.Resizable = false;

		m_Window = std::make_unique<Window>(winSpec);

		// Vulkan context
		m_Context = std::make_unique<VulkanContext>();

		// Vulkan swapchain
		m_Swapchain = std::make_unique<VulkanSwapchain>();

		// Vulkan renderer
		m_Renderer = std::make_unique<VulkanRenderer>();
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose())
		{
			m_Window->OnUpdate();
		}
	}

	void Application::Shutdown()
	{
		m_Renderer->Shutdown();
		m_Swapchain->Shutdown();
		m_Context->Shutdown();
		m_Window->Shutdown();
	}

}