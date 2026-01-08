#pragma once

#include <vulkan/vulkan.h>
#include "VulkanAbstraction/VulkanSwapchain.h"

namespace VulkanEngine {

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer() = default;
		ImGuiRenderer(const ImGuiRenderer&)				= delete;
		ImGuiRenderer& operator=(const ImGuiRenderer&)	= delete;

		void BeginImGuiFrame();
		void EndImGuiFrame(VkCommandBuffer cmd, AllocatedImage& swapchainImage);

	private:
		void InitImGuiCore();
		void InitImGuiPool();

		static void CheckVkResult(VkResult res);

	private:
		VkDescriptorPool m_ImGuiPool{ VK_NULL_HANDLE };
	};

}