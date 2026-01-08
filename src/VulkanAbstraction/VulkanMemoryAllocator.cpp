#include "VulkanAbstraction/VulkanMemoryAllocator.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>


namespace VulkanEngine {
	
	VulkanMemoryAllocator::VulkanMemoryAllocator()
	{
		auto* ctx = VulkanContext::GetRaw();

		VmaAllocatorCreateInfo allocatorInfo{
			.flags				= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.physicalDevice		= *ctx->GetPhysicalDevice(),
			.device				= *ctx->GetDevice(),
			.instance			= *ctx->GetInstance(),
			.vulkanApiVersion	= VK_API_VERSION_1_4
		};

		CHECK_VK_RES(vmaCreateAllocator(&allocatorInfo, &m_Allocator));

		// Deletor
		auto* app = Application::GetRaw();
		app->GetLifetimeManager()->Push(vmaDestroyAllocator, m_Allocator);
	}

	void VulkanMemoryAllocator::AllocateImage(
		VkImageCreateInfo	imageInfo,	VmaAllocationCreateInfo allocInfo,
		VkImage*			image,		VmaAllocation*			allocation)
	{
		CHECK_VK_RES(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, image, allocation, nullptr));
	}

}