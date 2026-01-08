#pragma once

#include <memory>
#include <string>

#include <VulkanEngine.h>


class MainLayer : public VulkanEngine::Layer
{
public:
	MainLayer(std::string layerName);
	virtual ~MainLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate() override;
	void OnEvent()  override;

private:
	// Descriptors
	std::shared_ptr<VulkanEngine::VulkanDescriptorSetAllocator> m_SetAllocator;
	std::shared_ptr<VulkanEngine::VulkanDescriptorSet>			m_Set;
	VkDescriptorSetLayout										m_SetLayout{ VK_NULL_HANDLE };

	// Shaders
	std::shared_ptr<VulkanEngine::VulkanShader> m_Shader;

	// Pipeline
	VkPipelineLayout	m_PipelineLayout{ VK_NULL_HANDLE };
	VkPipeline			m_Pipeline{ VK_NULL_HANDLE };
};