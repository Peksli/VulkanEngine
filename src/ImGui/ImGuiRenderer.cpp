#include "ImGuiRenderer.h"

#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vk_enum_string_helper.h>


namespace VulkanEngine {

	void ImGuiRenderer::CheckVkResult(VkResult res)
	{
		if (res != VK_SUCCESS)
		{
			VulkanEngine_CRITICAL(fmt::runtime("Vulkan ImGui Error: {0}"), string_VkResult(res));
			abort();
		}
	}

	ImGuiRenderer::ImGuiRenderer()
	{
		InitImGuiCore();
	}

	void ImGuiRenderer::BeginImGuiFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
	}

	void ImGuiRenderer::EndImGuiFrame(VkCommandBuffer cmd, AllocatedImage& renderTarget)
	{
		ImGui::Render();

		VulkanUtils::InsertImageMemoryBarrier(
			cmd,
			renderTarget.image,
			renderTarget.imageState,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		const VkRenderingAttachmentInfo colorAttachment = VulkanUtils::GetRenderingAttachmentInfo(
			renderTarget.imageView,
			nullptr,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		const VkRenderingInfo renderingInfo = VulkanUtils::GetRenderingInfo(
			colorAttachment,
			renderTarget.extent
		);

		vkCmdBeginRendering(cmd, &renderingInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
		vkCmdEndRendering(cmd);
	}

	void ImGuiRenderer::InitImGuiPool()
	{
		auto* ctx		 = VulkanContext::GetRaw();
		VkDevice device  = *ctx->GetDevice();

		constexpr uint32_t kPoolSize = 1000;

		const std::array<VkDescriptorPoolSize, 11> poolSizes = { {
			{.type = VK_DESCRIPTOR_TYPE_SAMPLER,                .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, .descriptorCount = kPoolSize },
			{.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       .descriptorCount = kPoolSize }
		} };

		const VkDescriptorPoolCreateInfo poolInfo = {
			.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets		= kPoolSize * static_cast<uint32_t>(poolSizes.size()),
			.poolSizeCount	= static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes		= poolSizes.data(),
		};

		CHECK_VK_RES(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_ImGuiPool));
	}

	void ImGuiRenderer::InitImGuiCore()
	{
		auto* app = Application::GetRaw();
		auto* ctx = VulkanContext::GetRaw();

		VkDevice device				= *ctx->GetDevice();
		VkPhysicalDevice physDev	= *ctx->GetPhysicalDevice();
		GLFWwindow* window			= app->GetWindow()->GetRaw();

		InitImGuiPool();
			
		// ImGui init
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui_ImplGlfw_InitForVulkan(window, true);

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, *ctx->GetSurface(), &capabilities);

		VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
		const VkPipelineRenderingCreateInfo pipelineRenderingInfo = {
			.sType						= VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount		= 1,
			.pColorAttachmentFormats	= &format,
		};

		ImGui_ImplVulkan_InitInfo initInfo = {
			.Instance			= *ctx->GetInstance(),
			.PhysicalDevice		= physDev,
			.Device				= device,
			.QueueFamily		= ctx->GetPhysicalDevice()->GetGraphicsFamily(),
			.Queue				= ctx->GetDevice()->GetGraphicsQueue(),
			.DescriptorPool		= m_ImGuiPool,
			.MinImageCount		= capabilities.minImageCount,
			.ImageCount			= capabilities.minImageCount + 1,
			.PipelineCache		= VK_NULL_HANDLE,
			.PipelineInfoMain	= {
				.MSAASamples					= VK_SAMPLE_COUNT_1_BIT,
				.PipelineRenderingCreateInfo	= pipelineRenderingInfo
			},
			.UseDynamicRendering = true,
			.CheckVkResultFn	 = &ImGuiRenderer::CheckVkResult
		};

		ImGui_ImplVulkan_Init(&initInfo);

		app->GetLifetimeManager()->Push(vkDestroyDescriptorPool, device, m_ImGuiPool, nullptr);
		app->GetLifetimeManager()->Push(ImGui_ImplVulkan_Shutdown);
	}

}