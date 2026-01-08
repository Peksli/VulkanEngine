#pragma once

#include <vulkan/vulkan.h>
#include "VulkanAbstraction/Shaders/VulkanShader.h"


namespace VulkanEngine {

	enum class PipelineType : uint8_t
	{
		Graphics,
		Compute
	};

	class VkPipelineBuilder
	{
	public:
		VkPipelineBuilder()				= default;
		virtual ~VkPipelineBuilder()	= default;

		VkPipelineBuilder& AddPipelineShader(std::shared_ptr<VulkanShader> shader);
		VkPipelineBuilder& AddPipelineLayout(VkPipelineLayout pipelineLayout);
		VkPipeline Build(PipelineType pipelineType);

	private:
		VkPipeline BuildCompute();

	private:
		std::shared_ptr<VulkanShader> m_Shader;
		VkPipelineLayout m_Layout{ VK_NULL_HANDLE };
	};

}