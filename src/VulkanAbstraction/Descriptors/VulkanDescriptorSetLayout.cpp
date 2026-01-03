#include "VulkanAbstraction/Descriptors/VulkanDescriptorSetLayout.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"


namespace VulkanEngine {

	// ------------------------------------------------------------------------------------
	// VULKAN DESCRIPTOR SET LAYOUT BUILDER
	// ------------------------------------------------------------------------------------

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::AddBinding(
		uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount)
	{
		VkDescriptorSetLayoutBinding bindingInfo{};
		bindingInfo.binding = binding;
		bindingInfo.descriptorType = descriptorType;
		bindingInfo.descriptorCount = descriptorCount;
		bindingInfo.stageFlags = VK_SHADER_STAGE_ALL;
		bindingInfo.pImmutableSamplers = nullptr;

		m_Bindings.push_back(bindingInfo);

		return *this;
	}

	VkDescriptorSetLayout VulkanDescriptorSetLayoutBuilder::Build()
	{
		auto* ctx = VulkanContext::GetRaw();
		VkDevice device = *ctx->GetDevice();

		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
		layoutInfo.pBindings = m_Bindings.data();

		VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
		CHECK_VK_RES(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

		return descriptorSetLayout;
	}

}