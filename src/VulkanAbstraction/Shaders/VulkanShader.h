#pragma once

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>
#include <vector>
#include <filesystem>

namespace VulkanEngine {

	class VulkanShader 
	{
	public:
		VulkanShader(std::filesystem::path shaderPath);
		virtual ~VulkanShader() = default;

		VkShaderModule GetRaw() const { return m_ShaderModule; }

	private:
		std::filesystem::path GetCacheDir()		const;
		std::filesystem::path GetCachedPath()	const;
		bool IsCacheValid()						const;
		void EnsureCacheDirExists();

		bool CompileOrLoad();
		bool CompileAndCache(const std::string& source);
		bool LoadFromCache();

		void CreateShaderModule();

	private:
		VkShaderModule			m_ShaderModule{ VK_NULL_HANDLE } ;
		std::filesystem::path	m_ShaderPath;
		std::vector<uint32_t>	m_SPIRV;
	};

}
