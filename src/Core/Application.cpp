#include "Core/Application.h"
#include "Core/LogSystem.h"


namespace VulkanEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
		: m_Spec(spec), m_LayerStack(std::make_unique<LayerStack>())
	{
		s_Instance = this;

		LogSystem::Initialize();

		m_LifetimeManager = std::make_unique<LifetimeManager>();

		WindowSpecification winSpec;
		winSpec.Width		= spec.windowWidth;
		winSpec.Height		= spec.windowHeight;
		winSpec.Title		= spec.windowName;
		winSpec.Resizable	= false;

		m_Window = std::make_unique<Window>(winSpec);
	}

	void Application::PushLayer(std::shared_ptr<Layer> layer)
	{
		m_LayerStack->PushLayer(layer);
	}

	void Application::PushOverlay(std::shared_ptr<Layer> overlay)
	{
		m_LayerStack->PushOverlay(overlay);
	}

	void Application::RemoveLayer(std::string layerName)
	{
		m_LayerStack->RemoveLayer(layerName);
	}

	void Application::RemoveOverlay(std::string overlayName)
	{
		m_LayerStack->RemoveOverlay(overlayName);
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose())
		{
			m_Window->OnUpdate();

			OnUpdate();
		}
	}

	void Application::OnUpdate()
	{
		for (auto it = m_LayerStack->begin(); it != m_LayerStack->end(); it++)
		{
			(*it)->OnUpdate();
		}
	}

	void Application::OnEvent()
	{
		for (auto it = m_LayerStack->rbegin(); it != m_LayerStack->rend(); it++)
		{
			(*it)->OnEvent(); // todo
		}
	}

	void Application::Shutdown()
	{
		m_LifetimeManager->Flush();
	}

}
