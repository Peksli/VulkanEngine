#pragma once

#include "Core/Layers/LayerStack.h"
#include "Core/LifetimeManager.h"
#include "Window/Window.h"


namespace VulkanEngine {

	struct ApplicationSpecification
	{
		std::string  windowName;
		int			 windowWidth;
		int			 windowHeight;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& spec);
		virtual ~Application() = default;

		void PushLayer(std::shared_ptr<Layer> layer);
		void PushOverlay(std::shared_ptr<Layer> overlay);
		void RemoveLayer(std::string layerName);
		void RemoveOverlay(std::string layerName);

		void Run();
		void Shutdown();
		void OnUpdate();
		void OnEvent();

		static Application*						GetRaw()					{ return s_Instance;		}
		const std::unique_ptr<Window>&			GetWindow()			 const	{ return m_Window;			}
		const std::unique_ptr<LifetimeManager>& GetLifetimeManager() const	{ return m_LifetimeManager; }

	private:
		static Application*					s_Instance;
		ApplicationSpecification			m_Spec;
		std::unique_ptr<Window>				m_Window;
		std::unique_ptr<LayerStack>			m_LayerStack;
		std::unique_ptr<LifetimeManager>	m_LifetimeManager;
	};

}
