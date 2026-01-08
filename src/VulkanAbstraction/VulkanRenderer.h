#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <memory>

#include "ImGui/ImGuiRenderer.h"

#include "VulkanAbstraction/Core/VulkanContext.h"
#include "VulkanAbstraction/VulkanSwapchain.h"
#include "VulkanAbstraction/VulkanMemoryAllocator.h"
#include "VulkanAbstraction/VulkanTypes.h" 

namespace VulkanEngine {

	static constexpr unsigned int FRAMES_IN_FLIGHT = 2;

	class VulkanRenderer
	{
	public:
		VulkanRenderer() = delete;
		~VulkanRenderer() = delete;

		static void Init();

		static void BeginFrame();
		static void EndFrame();
		static void BeginImGui();
		static void EndImGui();

		static void Clear(const glm::vec3& clearColor);
		static void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint);
		static void BindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, VkDescriptorSet set);
		static void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		static void EndInit();

		[[nodiscard]] static const VulkanContext& GetContext() { return *s_Context; }
		[[nodiscard]] static const AllocatedImage& GetRenderTarget() { return s_RenderTarget; }
		[[nodiscard]] static const VulkanMemoryAllocator& GetAllocator() { return *s_Allocator; }

	private:
		static void InitCore();
		static void InitRenderTarget();
		static void InitFrameData();
		static void InitSyncObjects();

		static void AdvanceFrame();

		static void BlitSceneToSwapchain(VkCommandBuffer cmd);

	private:
		static inline std::unique_ptr<VulkanContext>			s_Context;
		static inline std::unique_ptr<VulkanMemoryAllocator>	s_Allocator;
		static inline std::unique_ptr<ImGuiRenderer>			s_ImGuiRenderer;

		static inline AllocatedImage s_RenderTarget;

		static inline std::array<Frame, FRAMES_IN_FLIGHT> s_Frames;
		static inline std::vector<VkSemaphore> s_RenderFinishedSemaphores;

		static inline uint32_t s_CurrentFrameIndex = 0;
		static inline uint32_t s_CurrentImageIndex = 0;
	};

}