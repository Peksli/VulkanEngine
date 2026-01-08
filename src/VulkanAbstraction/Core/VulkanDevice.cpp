#include "VulkanAbstraction/Core/VulkanDevice.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

	VulkanDevice::VulkanDevice()
	{
		auto* ctx = VulkanContext::GetRaw();
		auto& physDevice = *ctx->GetPhysicalDevice();

		std::set<uint32_t> uniqueQueueFamilies = {
			physDevice.GetGraphicsFamily(),
			physDevice.GetPresentationFamily()
		};

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		float queuePriority = 1.0f;

		for (uint32_t familyIndex : uniqueQueueFamilies)
		{
			queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = familyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
				});
		}

		// Enable Features
		VkPhysicalDeviceVulkan12Features features12 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = nullptr,
			.descriptorIndexing  = VK_TRUE,
			.bufferDeviceAddress = VK_TRUE
		};

		VkPhysicalDeviceVulkan13Features features13 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &features12,
			.synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE
		};

		VkPhysicalDeviceFeatures2 features2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features13,
		};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// Create Logical Device
		VkDeviceCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features2,
			.flags = 0,
			.queueCreateInfoCount	 = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos		 = queueCreateInfos.data(),
			.enabledExtensionCount	 = static_cast<uint32_t>(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data(),
		};

		CHECK_VK_RES(vkCreateDevice(physDevice.GetRaw(), &createInfo, nullptr, &m_Device));

		// Retrieve Queues
		vkGetDeviceQueue(m_Device, physDevice.GetGraphicsFamily(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, physDevice.GetPresentationFamily(), 0, &m_PresentationQueue);

		// Deletor
		if (m_Device != VK_NULL_HANDLE)
		{
			auto* app = Application::GetRaw();
			app->GetLifetimeManager()->Push(vkDestroyDevice, m_Device, nullptr);
		}
	}

}