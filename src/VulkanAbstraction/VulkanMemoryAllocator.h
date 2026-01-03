#pragma once

#include <vma/vk_mem_alloc.h>


namespace VulkanEngine {

	class VulkanMemoryAllocator
	{
	public:
		VulkanMemoryAllocator();
		virtual ~VulkanMemoryAllocator() = default;

		void AllocateImage(VkImageCreateInfo imageInfo, VmaAllocationCreateInfo allocInfo, VkImage* image, VmaAllocation* allocation);

		VmaAllocator GetRaw() { return m_Allocator; }

	private:
		VmaAllocator m_Allocator{ VK_NULL_HANDLE };
	};

}