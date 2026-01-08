#pragma once

#include <vulkan/vulkan.h>


namespace VulkanEngine {

	class VkPipelineLayoutBuilder
	{
	public:
		VkPipelineLayoutBuilder()			= default;
		virtual ~VkPipelineLayoutBuilder()	= default;

		VkPipelineLayoutBuilder& AddDescriptorSetLayout(VkDescriptorSetLayout layout);
		VkPipelineLayout Build();

	private:
		VkDescriptorSetLayout m_Layout{ VK_NULL_HANDLE };
	};

}