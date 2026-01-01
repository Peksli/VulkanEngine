#include "Core/Application.h"


namespace VulkanEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
	{
		s_Instance = this;

		// GLFW window
		WindowSpecification winSpec;
		winSpec.Width = spec.width;
		winSpec.Height = spec.height;
		winSpec.Title = spec.windowName;
		winSpec.Resizable = false;

		m_Window = std::make_unique<Window>(winSpec);

		// Vulkan context
		m_Context = std::make_unique<VulkanContext>();

		// Vulkan swapchain
		m_Swapchain = std::make_unique<VulkanSwapchain>();
	}

	Application::~Application()
	{
		m_Swapchain->Shutdown();
		m_Context->Shutdown(); 
		m_Window->Shutdown();
	}

	void Application::Run()
	{
		while (true)
		{
			m_Window->OnUpdate();
		}
	}

}