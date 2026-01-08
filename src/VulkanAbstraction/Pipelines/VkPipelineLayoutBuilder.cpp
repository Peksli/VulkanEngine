#include "VulkanAbstraction/Pipelines/VkPipelineLayoutBuilder.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

	VkPipelineLayoutBuilder& VkPipelineLayoutBuilder::AddDescriptorSetLayout(VkDescriptorSetLayout layout)
	{
		m_Layout = layout;
		return *this;
	}

	VkPipelineLayout VkPipelineLayoutBuilder::Build()
	{
		auto* app = Application::GetRaw();
		auto* ctx = VulkanContext::GetRaw();
		VkDevice	device = *ctx->GetDevice();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext					= nullptr,
			.flags					= 0,
			.setLayoutCount			= 1,
			.pSetLayouts			= &m_Layout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges	= nullptr
		};

		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreatePipelineLayout(*ctx->GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

		app->GetLifetimeManager()->Push(vkDestroyPipelineLayout, device, pipelineLayout, nullptr);

		return pipelineLayout;
	}

}