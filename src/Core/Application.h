#pragma once

#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Window/Window.h"


namespace VulkanEngine
{

	struct ApplicationSpecification
	{
		std::string windowName;
		int width;
		int height;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& spec);
		virtual ~Application();

		void Run();

		static Application* GetRaw() { return s_Instance; }
		const std::unique_ptr<VulkanContext>&	GetContext()	const { return m_Context;	}
		const std::unique_ptr<Window>&			GetWindow()		const { return m_Window;	}

	private:
		static Application*				s_Instance;
		std::unique_ptr<Window>			m_Window;
		std::unique_ptr<VulkanContext>	m_Context;
	};

}