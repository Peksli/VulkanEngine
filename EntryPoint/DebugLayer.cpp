#include "DebugLayer.h"
#include "Core/LogSystem.h"
#include "MainLayer.h"


DebugLayer::DebugLayer(std::string name)
	: VulkanEngine::Layer(name)
{

}

void DebugLayer::OnAttach()
{
	Client_INFO(fmt::runtime("Layer {}: attached!"), m_Name);
}

void DebugLayer::OnDetach()
{
	Client_INFO(fmt::runtime("Layer {}: detached!"), m_Name);
}

void DebugLayer::OnUpdate()
{
	VulkanEngine::VulkanRenderer::BeginImGui();
	VulkanEngine::VulkanRenderer::EndImGui();

	VulkanEngine::VulkanRenderer::EndFrame();
}

void DebugLayer::OnEvent()
{

}