#include "VulkanAbstraction/Pipelines/VkPipelineBuilder.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

	VkPipelineBuilder& VkPipelineBuilder::AddPipelineShader(std::shared_ptr<VulkanShader> shader)
	{
		m_Shader = shader;
		return *this;
	}

	VkPipelineBuilder& VkPipelineBuilder::AddPipelineLayout(VkPipelineLayout pipelineLayout)
	{
		m_Layout = pipelineLayout;
		return *this;
	}

	VkPipeline VkPipelineBuilder::Build(PipelineType pipelineType)
	{
		switch (pipelineType)
		{
		case PipelineType::Compute: return BuildCompute();
		default:
			VulkanEngine_ERROR("Unknown pipeline type");
			std::unreachable();
		}
	}

	VkPipeline VkPipelineBuilder::BuildCompute()
	{
		auto*		app		= Application::GetRaw();
		auto*		ctx		= VulkanContext::GetRaw();
		VkDevice	device	= *ctx->GetDevice();

		// Stage info
		VkPipelineShaderStageCreateInfo stageInfo{
			.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext	= nullptr,
			.flags	= 0,
			.stage	= VK_SHADER_STAGE_COMPUTE_BIT,
			.module = m_Shader->GetRaw(),
			.pName	= "main"
		};

		// Pipeline
		VkComputePipelineCreateInfo computePipelineCreateInfo{
			.sType				= VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext				= nullptr,
			.flags				= 0,
			.stage				= stageInfo,
			.layout				= m_Layout,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex	= -1
		};

		VkPipeline pipeline{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateComputePipelines(*ctx->GetDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline));

		app->GetLifetimeManager()->Push(vkDestroyPipeline, device, pipeline, nullptr);

		return pipeline;
	}

}