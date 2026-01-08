#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <filesystem>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>

#include "Core/LogSystem.h"
#include "VulkanAbstraction/VulkanRenderer.h"

#define CHECK_VK_RES(res) \
    if(res != VK_SUCCESS) { \
        VulkanEngine_CRITICAL(fmt::runtime("Vulkan Validation Error: {0}"), string_VkResult(res)); \
        abort(); \
    }

namespace VulkanEngine {

	namespace FilesystemUtils
	{
		enum class ReadMode : uint8_t
		{
			Binary,
			Text
		};

		std::string ReadFile(const std::filesystem::path& filepath, ReadMode mode);
	}

	namespace VulkanUtils
	{
		// SYNC
		VkSemaphoreSubmitInfo GetSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
		VkFenceCreateInfo GetFenceCreateInfo(VkFenceCreateFlags flags = 0);
		VkSemaphoreCreateInfo GetSemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

		void InsertImageMemoryBarrier(
			VkCommandBuffer       cmdBuffer, VkImage        image,
			ImageState& imageState,   // expands into srcAccess + stage
			VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
			VkImageLayout         newLayout);

		// COMMAND BUFFER + POOL
		VkCommandPoolCreateInfo GetCommandPoolInfo(VkCommandPoolCreateFlags flags = 0, uint32_t queueFamilyIndex = 0);
		VkCommandBufferAllocateInfo GetCmdBufferAllocateInfo(VkCommandPool cmdPool, uint32_t cmdBufferCount);
		VkCommandBufferBeginInfo GetBeginCmdBufferInfo(VkCommandBufferUsageFlags flags = 0);

		// IMAGES
		VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags aspect);
		VkImageCreateInfo GetImageCreateInfo(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage);
		VkImageViewCreateInfo GetImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

		// TRANSFER
		void CopyImageToImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent3D srcSize, VkExtent3D dstSize);

		// DYNAMIC RENDERING
		VkRenderingAttachmentInfo GetRenderingAttachmentInfo(VkImageView imageView, VkClearValue* clearValue, VkImageLayout imageLayout);
		VkRenderingInfo GetRenderingInfo(VkRenderingAttachmentInfo colorAttachmentInfo, VkExtent3D extent);

		// SUBMIT + PRESENT
		VkCommandBufferSubmitInfo GetCommandBufferSubmitInfo(VkCommandBuffer cmd);
		VkSubmitInfo2 GetSubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
	}
}