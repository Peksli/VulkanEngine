#pragma once

#include <string>


namespace VulkanEngine {

	class Layer
	{
	public:
		Layer(std::string name);
		virtual ~Layer() = default;

		virtual void OnAttach();
		virtual void OnDetach();
		virtual void OnUpdate();
		virtual void OnEvent();

		std::string GetName() const { return m_Name; }

	protected:
		std::string m_Name = "Unknown";
	};

}