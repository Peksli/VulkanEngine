#include "Core/Layers/Layer.h"
#include "Core/LogSystem.h"


namespace VulkanEngine {

	Layer::Layer(std::string name)
		: m_Name(name)
	{

	}

	// ===========================================================================
	// Default behavior
	// ===========================================================================

	void Layer::OnAttach()
	{
		VulkanEngine_INFO(fmt::runtime("Layer: {0} attached!"), m_Name);
	}

	void Layer::OnDetach()
	{
		VulkanEngine_INFO(fmt::runtime("Layer: {0} detached!"), m_Name);
	}

	void Layer::OnUpdate()
	{

	}

	void Layer::OnEvent()
	{

	}

}