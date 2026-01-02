#include "Utility/Utility.h"


namespace VulkanEngine {

	namespace VulkanUtils
	{
		// SYNC
		VkSemaphoreSubmitInfo GetSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
		{
			VkSemaphoreSubmitInfo semaphoreSubmitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
			semaphoreSubmitInfo.pNext = nullptr;
			semaphoreSubmitInfo.semaphore = semaphore;
			semaphoreSubmitInfo.stageMask = stageMask;
			semaphoreSubmitInfo.deviceIndex = 0;
			semaphoreSubmitInfo.value = 1;

			return semaphoreSubmitInfo;
		}

		VkFenceCreateInfo GetFenceCreateInfo(VkFenceCreateFlags flags)
		{
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.pNext = nullptr;
			fenceInfo.flags = flags;

			return fenceInfo;
		}

		VkSemaphoreCreateInfo GetSemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
		{
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext = nullptr;
			semaphoreInfo.flags = flags;

			return semaphoreInfo;
		}

		void InsertImageMemoryBarrier(
			VkCommandBuffer       cmdBuffer,    VkImage        image,
			ImageState&           imageState,   // expands into srcAccess + stage
			VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
			VkImageLayout		  newLayout)
		{
			// Make image mem (refered by src combo) 1) available and 2) visible to the combo of dst
			// Available - in L2 cache, visible - in corresponding clusters with L1 caches
			VkImageMemoryBarrier2 imageBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			imageBarrier.pNext			= nullptr;
			imageBarrier.srcStageMask	= imageState.currentStage;
			imageBarrier.srcAccessMask	= imageState.currentAccess;
			imageBarrier.dstStageMask	= dstStageMask;
			imageBarrier.dstAccessMask	= dstAccessMask;
			imageBarrier.oldLayout		= imageState.currentLayout;
			imageBarrier.newLayout		= newLayout;
			imageBarrier.image			= image;
			imageBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.baseArrayLayer	= 0;
			imageBarrier.subresourceRange.baseMipLevel		= 0;
			imageBarrier.subresourceRange.layerCount		= 1;
			imageBarrier.subresourceRange.levelCount		= 1;

			VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
			depInfo.pNext = nullptr;
			depInfo.imageMemoryBarrierCount = 1;
			depInfo.pImageMemoryBarriers = &imageBarrier;

			vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

			imageState.currentStage = dstStageMask;
			imageState.currentAccess = dstAccessMask;
			imageState.currentLayout = newLayout;
		}

		// COMMAND BUFFER + POOL
		VkCommandPoolCreateInfo GetCommandPoolInfo(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.pNext = nullptr;
			poolInfo.flags = flags;
			poolInfo.queueFamilyIndex = queueFamilyIndex;

			return poolInfo;
		}

		VkCommandBufferAllocateInfo GetCmdBufferAllocateInfo(VkCommandPool cmdPool, uint32_t cmdBufferCount)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = cmdPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = cmdBufferCount;

			return allocInfo;
		}

		VkCommandBufferBeginInfo GetBeginCmdBufferInfo(VkCommandBufferUsageFlags flags)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.pInheritanceInfo = nullptr;
			beginInfo.flags = flags;

			return beginInfo;
		}

		// IMAGES
		VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags aspect)
		{
			VkImageSubresourceRange range{};
			range.aspectMask		= aspect;
			range.baseMipLevel		= 0;
			range.levelCount		= 1;
			range.baseArrayLayer	= 0;
			range.layerCount		= 1;

			return range;
		}

		// SUBMIT + PRESENT
		VkCommandBufferSubmitInfo GetCommandBufferSubmitInfo(VkCommandBuffer cmd)
		{
			VkCommandBufferSubmitInfo cmdBufferSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
			cmdBufferSubmitInfo.pNext			= nullptr;
			cmdBufferSubmitInfo.commandBuffer	= cmd;
			cmdBufferSubmitInfo.deviceMask		= 0;

			return cmdBufferSubmitInfo;
		}

		VkSubmitInfo2 GetSubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
		{
			VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
			submitInfo.pNext					= nullptr;
			submitInfo.waitSemaphoreInfoCount	= 1;
			submitInfo.pWaitSemaphoreInfos		= waitSemaphoreInfo;
			submitInfo.signalSemaphoreInfoCount = 1;
			submitInfo.pSignalSemaphoreInfos	= signalSemaphoreInfo;
			submitInfo.commandBufferInfoCount	= 1;
			submitInfo.pCommandBufferInfos		= cmd;

			return submitInfo;
		}
	}

}