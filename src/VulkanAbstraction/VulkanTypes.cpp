#include "VulkanAbstraction/VulkanTypes.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

	void Frame::Init(VkDevice device, uint32_t queueFamilyIndex)
	{
		auto* ctx = VulkanContext::GetRaw();
		auto* app = Application::GetRaw();

		// Command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; 
		poolInfo.queueFamilyIndex = queueFamilyIndex;

		CHECK_VK_RES(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
		app->GetLifetimeManager()->Push(vkDestroyCommandPool, device, commandPool, nullptr);

		// Command buffer
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		CHECK_VK_RES(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

		// Fence
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		CHECK_VK_RES(vkCreateFence(device, &fenceInfo, nullptr, &renderFinishedFence));
		app->GetLifetimeManager()->Push(vkDestroyFence, device, renderFinishedFence, nullptr);

		// Semaphore
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		CHECK_VK_RES(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore));
		app->GetLifetimeManager()->Push(vkDestroySemaphore, device, imageAvailableSemaphore, nullptr);
	}


}