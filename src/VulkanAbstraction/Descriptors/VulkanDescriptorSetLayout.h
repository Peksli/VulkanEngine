#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>


namespace VulkanEngine {

	class VulkanDescriptorSetLayoutBuilder
	{
	public:
		VulkanDescriptorSetLayoutBuilder() = default;
		virtual ~VulkanDescriptorSetLayoutBuilder() = default;

		VulkanDescriptorSetLayoutBuilder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount);
		VkDescriptorSetLayout Build();

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
	};

}