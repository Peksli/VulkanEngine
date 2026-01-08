#include "VulkanAbstraction/Descriptors/VulkanDescriptorSetAllocator.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

	VulkanDescriptorSetAllocator::VulkanDescriptorSetAllocator(uint32_t maxSets, const std::vector<PoolSize>& poolSizes)
	{
		auto* app = Application::GetRaw();
		auto* ctx = VulkanContext::GetRaw();
		VkDevice device = *ctx->GetDevice();

		// Pool info
		std::vector<VkDescriptorPoolSize> vkPoolSizes;
		vkPoolSizes.reserve(poolSizes.size());

		for (const auto& poolSize : poolSizes)
		{
			vkPoolSizes.push_back(VkDescriptorPoolSize{
				.type				= poolSize.descriptorType,
				.descriptorCount	= poolSize.descriptorCount
				});
		}

		VkDescriptorPoolCreateInfo poolInfo{
			.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext			= nullptr,
			.flags			= 0,
			.maxSets		= maxSets,
			.poolSizeCount	= static_cast<uint32_t>(vkPoolSizes.size()),
			.pPoolSizes		= vkPoolSizes.data()
		};

		// Creation
		CHECK_VK_RES(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_Pool));

		// Deletor
		app->GetLifetimeManager()->Push(vkDestroyDescriptorPool, device, m_Pool, nullptr);
	}

	std::shared_ptr<VulkanDescriptorSet> VulkanDescriptorSetAllocator::Allocate(VkDescriptorSetLayout layout)
	{
		auto*		ctx		= VulkanContext::GetRaw();
		VkDevice	device	= *ctx->GetDevice();

		VkDescriptorSetAllocateInfo allocateInfo{
			.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext				= nullptr,
			.descriptorPool		= m_Pool,
			.descriptorSetCount = 1,
			.pSetLayouts		= &layout
		};

		VkDescriptorSet set{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkAllocateDescriptorSets(device, &allocateInfo, &set));

		return std::make_shared<VulkanDescriptorSet>(set);
	}

	void VulkanDescriptorSetAllocator::Reset()
	{
		auto* ctx = VulkanContext::GetRaw();
		VkDevice device = *ctx->GetDevice();

		CHECK_VK_RES(vkResetDescriptorPool(device, m_Pool, 0));
	}

}