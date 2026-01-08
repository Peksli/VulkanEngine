#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>


namespace VulkanEngine {

	class VkDescriptorSetLayoutBuilder
	{
	public:
		VkDescriptorSetLayoutBuilder()			= default;
		virtual ~VkDescriptorSetLayoutBuilder() = default;

		VkDescriptorSetLayoutBuilder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount);
		VkDescriptorSetLayout Build();

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
	};

}