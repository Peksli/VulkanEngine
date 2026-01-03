#include "Core/Application.h"
#include "Core/LogSystem.h"


namespace VulkanEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
	{
		s_Instance = this;

		// Log system
		LogSystem::Initialize();

		// Lifetime manager
		m_LifetimeManager = std::make_unique<LifetimeManager>();

		// GLFW window
		WindowSpecification winSpec;
		winSpec.Width		= spec.width;
		winSpec.Height		= spec.height;
		winSpec.Title		= spec.windowName;
		winSpec.Resizable	= false;

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
			m_Renderer->OpenRenderScope();
			m_Renderer->Clear({ 1.0f, 0.0f, 0.0f });
			m_Renderer->EndRenderScope();
		}
	}

	void Application::Shutdown()
	{
		m_LifetimeManager->Flush();
	}

}