#include "Utility/Utility.h"

namespace VulkanEngine {

	namespace FilesystemUtils
	{
		std::string ReadFile(const std::filesystem::path& filepath, ReadMode mode)
		{
			if (!std::filesystem::exists(filepath))
			{
				VulkanEngine_ERROR(fmt::runtime("File does not exist: {}"), filepath.string());
				return {};
			}

			std::ios::openmode openMode = std::ios::in;
			if (mode == ReadMode::Binary)
				openMode |= std::ios::binary;

			std::ifstream file(filepath, openMode);
			if (!file.is_open())
			{
				VulkanEngine_ERROR(fmt::runtime("Failed to open file: {}"), filepath.string());
				return {};
			}

			if (mode == ReadMode::Binary)
			{
				file.seekg(0, std::ios::end);
				std::string result;
				result.resize(file.tellg());
				file.seekg(0, std::ios::beg);
				file.read(result.data(), result.size());
				return result;
			}
			else
			{
				std::stringstream buffer;
				buffer << file.rdbuf();
				return buffer.str();
			}
		}
	}

	namespace VulkanUtils
	{
		// -----------------------------------------------------------------------------------------------------------
		// SYNC
		// -----------------------------------------------------------------------------------------------------------

		VkSemaphoreSubmitInfo GetSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.pNext = nullptr,
				.semaphore = semaphore,
				.value = 1,
				.stageMask = stageMask,
				.deviceIndex = 0,
			};
		}

		VkFenceCreateInfo GetFenceCreateInfo(VkFenceCreateFlags flags)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = flags
			};
		}

		VkSemaphoreCreateInfo GetSemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = flags
			};
		}

		void InsertImageMemoryBarrier(
			VkCommandBuffer       cmdBuffer, VkImage        image,
			ImageState& imageState,
			VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
			VkImageLayout         newLayout)
		{
			VkImageMemoryBarrier2 imageBarrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = imageState.currentStage,
				.srcAccessMask = imageState.currentAccess,
				.dstStageMask = dstStageMask,
				.dstAccessMask = dstAccessMask,
				.oldLayout = imageState.currentLayout,
				.newLayout = newLayout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = image,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};

			const VkDependencyInfo depInfo{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = nullptr,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &imageBarrier
			};

			vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

			imageState.currentStage = dstStageMask;
			imageState.currentAccess = dstAccessMask;
			imageState.currentLayout = newLayout;
		}

		// -----------------------------------------------------------------------------------------------------------
		// COMMAND BUFFER + POOL
		// -----------------------------------------------------------------------------------------------------------

		VkCommandPoolCreateInfo GetCommandPoolInfo(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.pNext = nullptr,
				.flags = flags,
				.queueFamilyIndex = queueFamilyIndex
			};
		}

		VkCommandBufferAllocateInfo GetCmdBufferAllocateInfo(VkCommandPool cmdPool, uint32_t cmdBufferCount)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = cmdPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = cmdBufferCount
			};
		}

		VkCommandBufferBeginInfo GetBeginCmdBufferInfo(VkCommandBufferUsageFlags flags)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = flags,
				.pInheritanceInfo = nullptr
			};
		}

		// -----------------------------------------------------------------------------------------------------------
		// IMAGES
		// -----------------------------------------------------------------------------------------------------------

		VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags aspect)
		{
			return {
				.aspectMask = aspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			};
		}

		VkImageCreateInfo GetImageCreateInfo(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format,
				.extent = extent,
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};
		}

		VkImageViewCreateInfo GetImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspectMask)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = format,
				.subresourceRange = {
					.aspectMask = aspectMask,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};
		}

		// -----------------------------------------------------------------------------------------------------------
		// TRANSFER
		// -----------------------------------------------------------------------------------------------------------

		void CopyImageToImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent3D srcSize, VkExtent3D dstSize)
		{
			VkImageBlit2 blitRegion{
				.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
				.pNext = nullptr,
				.srcSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
				.srcOffsets = {
					{0, 0, 0},
					{static_cast<int32_t>(srcSize.width), static_cast<int32_t>(srcSize.height), static_cast<int32_t>(srcSize.depth)}
				},
				.dstSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
				.dstOffsets = {
					{0, 0, 0},
					{static_cast<int32_t>(dstSize.width), static_cast<int32_t>(dstSize.height), static_cast<int32_t>(dstSize.depth)}
				}
			};

			const VkBlitImageInfo2 blitInfo{
				.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
				.pNext = nullptr,
				.srcImage = src,
				.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.dstImage = dst,
				.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.regionCount = 1,
				.pRegions = &blitRegion,
				.filter = VK_FILTER_LINEAR,
			};

			vkCmdBlitImage2(cmdBuffer, &blitInfo);
		}

		// -----------------------------------------------------------------------------------------------------------
		// DYNAMIC RENDERING
		// -----------------------------------------------------------------------------------------------------------

		VkRenderingAttachmentInfo GetRenderingAttachmentInfo(VkImageView imageView, VkClearValue* clearValue, VkImageLayout imageLayout)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.pNext = nullptr,
				.imageView = imageView,
				.imageLayout = imageLayout,
				.loadOp = clearValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = clearValue ? *clearValue : VkClearValue{}
			};
		}

		VkRenderingInfo GetRenderingInfo(VkRenderingAttachmentInfo colorAttachmentInfo, VkExtent3D extent)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.pNext = nullptr,
				.flags = 0,
				.renderArea = {.offset = {0, 0}, .extent = {extent.width, extent.height} },
				.layerCount = 1,
				.viewMask = 0,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttachmentInfo,
				.pDepthAttachment = nullptr,
				.pStencilAttachment = nullptr
			};
		}

		// -----------------------------------------------------------------------------------------------------------
		// SUBMIT + PRESENT
		// -----------------------------------------------------------------------------------------------------------

		VkCommandBufferSubmitInfo GetCommandBufferSubmitInfo(VkCommandBuffer cmd)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				.pNext = nullptr,
				.commandBuffer = cmd,
				.deviceMask = 0
			};
		}

		VkSubmitInfo2 GetSubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
		{
			return {
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.pNext = nullptr,
				.waitSemaphoreInfoCount = waitSemaphoreInfo ? 1u : 0u,
				.pWaitSemaphoreInfos = waitSemaphoreInfo,
				.commandBufferInfoCount = 1,
				.pCommandBufferInfos = cmd,
				.signalSemaphoreInfoCount = signalSemaphoreInfo ? 1u : 0u,
				.pSignalSemaphoreInfos = signalSemaphoreInfo
			};
		}
	}
}