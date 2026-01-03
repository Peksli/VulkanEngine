#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>


namespace VulkanEngine {

    struct ImageState
    {
        VkPipelineStageFlags2 currentStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        VkAccessFlags2        currentAccess = 0;
        VkImageLayout         currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

	struct AllocatedImage
	{
        ImageState      imageState;
		VkImage			image;
		VkImageView		imageView;
		VkFormat		format;
		VkExtent3D		extent;
		VmaAllocation	allocation;
	};

    struct Frame
    {
        VkCommandPool   commandPool{ VK_NULL_HANDLE };
        VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
        VkFence         renderFinishedFence{ VK_NULL_HANDLE };
        VkSemaphore     imageAvailableSemaphore{ VK_NULL_HANDLE };

        void Init(VkDevice device, uint32_t queueFamilyIndex);
    };

}