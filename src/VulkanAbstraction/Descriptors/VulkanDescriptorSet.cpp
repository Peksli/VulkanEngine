#include "VulkanAbstraction/Descriptors/VulkanDescriptorSet.h"
#include "VulkanAbstraction/Core/VulkanContext.h"


namespace VulkanEngine {

	VulkanDescriptorSet::VulkanDescriptorSet(VkDescriptorSet set)
		: m_Set(set)
	{

	}

	void VulkanDescriptorSet::WriteImage(VkImageView imageView, VkImageLayout imageLayout, uint32_t dstBinding)
	{
		VkDescriptorImageInfo imageInfo{
			.imageView		= imageView,
			.imageLayout	= imageLayout
		};

		VkWriteDescriptorSet imageWrite{
			.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext				= nullptr,
			.dstSet				= m_Set,
			.dstBinding			= dstBinding,
			.descriptorCount	= 1,
			.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo			= &imageInfo
		};

		auto* ctx = VulkanContext::GetRaw();
		vkUpdateDescriptorSets(*ctx->GetDevice(), 1, &imageWrite, 0, nullptr);
	}
}