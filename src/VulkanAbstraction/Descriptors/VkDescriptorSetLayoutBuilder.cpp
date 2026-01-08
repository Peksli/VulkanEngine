#include "VulkanAbstraction/Descriptors/VkDescriptorSetLayoutBuilder.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

	VkDescriptorSetLayoutBuilder& VkDescriptorSetLayoutBuilder::AddBinding(
		uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount)
	{
		m_Bindings.push_back(VkDescriptorSetLayoutBinding{
			.binding			= binding,
			.descriptorType		= descriptorType,
			.descriptorCount	= descriptorCount,
			.stageFlags			= VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
			});

		return *this;
	}

	VkDescriptorSetLayout VkDescriptorSetLayoutBuilder::Build()
	{
		auto*		app		= Application::GetRaw();
		auto*		ctx		= VulkanContext::GetRaw();
		VkDevice	device	= *ctx->GetDevice();

		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext			= nullptr,
			.flags			= 0,
			.bindingCount	= static_cast<uint32_t>(m_Bindings.size()),
			.pBindings		= m_Bindings.data()
		};

		VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

		app->GetLifetimeManager()->Push(vkDestroyDescriptorSetLayout, device, descriptorSetLayout, nullptr);

		return descriptorSetLayout;
	}

}