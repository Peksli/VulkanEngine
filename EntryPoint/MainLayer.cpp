#include "MainLayer.h"


MainLayer::MainLayer(std::string layerName)
	: VulkanEngine::Layer(layerName)
{

}

void MainLayer::OnAttach()
{
	VulkanEngine::VulkanRenderer::Init();

	Client_INFO(fmt::runtime("Layer {}: attached!"), m_Name);

	std::filesystem::current_path("F:\\Langs\\C++\\Petprojects\\VulkanEngine\\EntryPoint");

	// Descriptors
	uint32_t maxSetsToAllocate = 1;
	std::vector<VulkanEngine::PoolSize> poolSizes = { {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1} };
	m_SetAllocator = std::make_shared<VulkanEngine::VulkanDescriptorSetAllocator>(maxSetsToAllocate, poolSizes);

	m_SetLayout = VulkanEngine::VkDescriptorSetLayoutBuilder()
		.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
		.Build();

	m_Set = m_SetAllocator->Allocate(m_SetLayout);
	m_Set->WriteImage(VulkanEngine::VulkanRenderer::GetRenderTarget().imageView, VK_IMAGE_LAYOUT_GENERAL, 0);

	// Shaders
	m_Shader = std::make_shared<VulkanEngine::VulkanShader>("Assets\\Shaders\\MyCompute.comp");

	// Pipeline
	m_PipelineLayout = VulkanEngine::VkPipelineLayoutBuilder()
		.AddDescriptorSetLayout(m_SetLayout)
		.Build();

	m_Pipeline = VulkanEngine::VkPipelineBuilder()
		.AddPipelineLayout(m_PipelineLayout)
		.AddPipelineShader(m_Shader)
		.Build(VulkanEngine::PipelineType::Compute);

	// End
	VulkanEngine::VulkanRenderer::EndInit();
}

void MainLayer::OnDetach()
{
	Client_INFO(fmt::runtime("Layer {}: detached!"), m_Name);
}

void MainLayer::OnUpdate()
{
	VulkanEngine::VulkanRenderer::BeginFrame();
	VulkanEngine::VulkanRenderer::BindPipeline(m_Pipeline, VK_PIPELINE_BIND_POINT_COMPUTE);
	VulkanEngine::VulkanRenderer::BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout, m_Set->GetRaw());
	VulkanEngine::VulkanRenderer::Dispatch(80, 45, 1);
}

void MainLayer::OnEvent()
{

}