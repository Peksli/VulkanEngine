#pragma once

#include <vulkan/vulkan.h>


namespace VulkanEngine {

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet(VkDescriptorSet set);
		virtual ~VulkanDescriptorSet() = default;

		void WriteImage(VkImageView imageView, VkImageLayout imageLayout, uint32_t dstBinding);

		VkDescriptorSet GetRaw() { return m_Set; }

	private:
		VkDescriptorSet m_Set{ VK_NULL_HANDLE };
	};

}