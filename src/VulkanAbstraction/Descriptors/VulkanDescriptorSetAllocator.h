#pragma once

#include "VulkanAbstraction/Descriptors/VkDescriptorSetLayoutBuilder.h"
#include "VulkanAbstraction/Descriptors/VulkanDescriptorSet.h"

#include <vulkan/vulkan.h>
#include <memory>


namespace VulkanEngine {

	struct PoolSize
	{
		VkDescriptorType descriptorType;
		uint32_t descriptorCount;
	};

	class VulkanDescriptorSetAllocator
	{
	public:
		VulkanDescriptorSetAllocator(uint32_t maxSets, const std::vector<PoolSize>& poolSizes);
		virtual ~VulkanDescriptorSetAllocator() = default;

		std::shared_ptr<VulkanDescriptorSet> Allocate(VkDescriptorSetLayout layout);
		void Reset();

		VkDescriptorPool GetRaw() const { return m_Pool; }

	private:
		VkDescriptorPool m_Pool{ VK_NULL_HANDLE };
	};

}