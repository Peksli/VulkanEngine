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

		VkImageCreateInfo GetImageCreateInfo(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage)
		{
			VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
			imageInfo.pNext = nullptr;
			imageInfo.flags = 0;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = format;
			imageInfo.extent = extent;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = usage;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.queueFamilyIndexCount = 0;
			imageInfo.pQueueFamilyIndices = nullptr;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			return imageInfo;
		}

		VkImageViewCreateInfo GetImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspectMask)
		{
			VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			imageViewInfo.pNext = nullptr;
			imageViewInfo.flags = 0;
			imageViewInfo.image = image;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewInfo.format = format;
			imageViewInfo.subresourceRange.aspectMask = aspectMask;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			imageViewInfo.subresourceRange.levelCount = 1;

			return imageViewInfo;
		}

		// TRANSFER
		void CopyImageToImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent3D srcSize, VkExtent3D dstSize)
		{
			VkImageBlit2 blitRegion{ VK_STRUCTURE_TYPE_IMAGE_BLIT_2 };
			blitRegion.pNext = nullptr;

			blitRegion.srcOffsets[1].x = srcSize.width;
			blitRegion.srcOffsets[1].y = srcSize.height;
			blitRegion.srcOffsets[1].z = srcSize.depth;

			blitRegion.dstOffsets[1].x = dstSize.width;
			blitRegion.dstOffsets[1].y = dstSize.height;
			blitRegion.dstOffsets[1].z = dstSize.depth;

			blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.srcSubresource.baseArrayLayer = 0;
			blitRegion.srcSubresource.layerCount = 1;
			blitRegion.srcSubresource.mipLevel = 0;

			blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.dstSubresource.baseArrayLayer = 0;
			blitRegion.dstSubresource.layerCount = 1;
			blitRegion.dstSubresource.mipLevel = 0;

			VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
			blitInfo.dstImage = dst;
			blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			blitInfo.srcImage = src;
			blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			blitInfo.filter = VK_FILTER_LINEAR;
			blitInfo.regionCount = 1;
			blitInfo.pRegions = &blitRegion;

			vkCmdBlitImage2(cmdBuffer, &blitInfo);
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