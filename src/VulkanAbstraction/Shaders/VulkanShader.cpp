#include "VulkanAbstraction/Shaders/VulkanShader.h"
#include "VulkanAbstraction/Core/VulkanContext.h"
#include "Core/Application.h"
#include "Core/LogSystem.h"
#include "Utility/Utility.h"

#include <fstream>
#include <filesystem>

namespace VulkanEngine {

	static shaderc_shader_kind GetShadercKind(const std::filesystem::path& shaderPath) 
	{
		std::string shaderExt = shaderPath.extension().string();

		if (shaderExt == ".vert") return shaderc_vertex_shader;
		if (shaderExt == ".frag") return shaderc_fragment_shader;
		if (shaderExt == ".comp") return shaderc_compute_shader;
		if (shaderExt == ".geom") return shaderc_geometry_shader;

		VulkanEngine_WARN(fmt::runtime("Unsupported shader extension: {}"), shaderExt);
		return shaderc_glsl_infer_from_source;
	}

	VulkanShader::VulkanShader(std::filesystem::path shaderPath)
		: m_ShaderPath(shaderPath)
	{
		if (m_ShaderPath.empty()) 
		{
			VulkanEngine_CRITICAL("Empty shader path");
			return;
		}

		CompileOrLoad();
		CreateShaderModule();

		// Deletor
		auto* ctx = VulkanContext::GetRaw();
		auto* app = Application::GetRaw();
		VkDevice device = *ctx->GetDevice();

		app->GetLifetimeManager()->Push(vkDestroyShaderModule, device, m_ShaderModule, nullptr);
	}

	std::filesystem::path VulkanShader::GetCacheDir() const 
	{
		return std::filesystem::current_path() / "Cache" / "Shaders";
	}

	std::filesystem::path VulkanShader::GetCachedPath() const 
	{
		return GetCacheDir() / (m_ShaderPath.stem().string() + ".spv");
	}

	bool VulkanShader::IsCacheValid() const 
	{
		auto cachePath = GetCachedPath();
		if (!std::filesystem::exists(cachePath)) return false;

		auto sourceTime  = std::filesystem::last_write_time(m_ShaderPath);
		auto cacheTime   = std::filesystem::last_write_time(cachePath);

		return cacheTime > sourceTime; 
	}

	void VulkanShader::EnsureCacheDirExists() 
	{
		std::filesystem::create_directories(GetCacheDir());
	}

	bool VulkanShader::CompileOrLoad() 
	{
		EnsureCacheDirExists();

		auto cachedPath = GetCachedPath();

		if (IsCacheValid()) 
		{
			VulkanEngine_DEBUG(fmt::runtime("Loading cached SPIR-V: {}"), cachedPath.string());
			if (LoadFromCache()) return true;
			VulkanEngine_WARN("Failed to load cache (corrupted?), recompiling...");
		}

		VulkanEngine_DEBUG(fmt::runtime("Compiling shader: {}"), m_ShaderPath.string());

		std::string source = FilesystemUtils::ReadFile(m_ShaderPath, FilesystemUtils::ReadMode::Text);
		if (source.empty()) 
		{
			VulkanEngine_CRITICAL(fmt::runtime("Failed to read shader source: {}"), m_ShaderPath.string());
			return false;
		}

		return CompileAndCache(source);
	}

	bool VulkanShader::CompileAndCache(const std::string& source) 
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
			source.c_str(), source.size(),
			GetShadercKind(m_ShaderPath),
			m_ShaderPath.string().c_str(),
			"main", options
		);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) 
		{
			VulkanEngine_CRITICAL(fmt::runtime("Shader compilation failed ({}):\n{}"), m_ShaderPath.string(), result.GetErrorMessage());
			return false;
		}

		m_SPIRV.assign(result.begin(), result.end());

		// Write cache
		std::ofstream cacheFile(GetCachedPath(), std::ios::binary | std::ios::trunc);
		if (cacheFile.is_open()) 
		{
			cacheFile.write(reinterpret_cast<const char*>(m_SPIRV.data()), m_SPIRV.size() * sizeof(uint32_t));
			VulkanEngine_DEBUG(fmt::runtime("Cached SPIR-V to: {}"), GetCachedPath().string());
		}

		return true;
	}

	bool VulkanShader::LoadFromCache() 
	{
		std::ifstream cacheFile(GetCachedPath(), std::ios::binary | std::ios::ate);
		if (!cacheFile.is_open()) return false;

		auto fileSize = static_cast<std::size_t>(cacheFile.tellg());
		if (fileSize <= 0 || fileSize % sizeof(uint32_t) != 0) 
		{
			VulkanEngine_WARN(fmt::runtime("Invalid SPIR-V cache file size: {}"), fileSize);
			return false;
		}

		cacheFile.seekg(0, std::ios::beg);
		m_SPIRV.resize(fileSize / sizeof(uint32_t));
		cacheFile.read(reinterpret_cast<char*>(m_SPIRV.data()), fileSize);

		return !m_SPIRV.empty();
	}

	void VulkanShader::CreateShaderModule()
	{
		if (m_SPIRV.empty())
		{
			VulkanEngine_CRITICAL(fmt::runtime("No SPIR-V data: {}"), m_ShaderPath.string());
		}

		auto* ctx = VulkanContext::GetRaw();

		VkShaderModuleCreateInfo createInfo{
			.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext		= nullptr,
			.flags		= 0,
			.codeSize	= m_SPIRV.size() * sizeof(uint32_t),
			.pCode		= m_SPIRV.data()
		};

		CHECK_VK_RES(vkCreateShaderModule(*ctx->GetDevice(), &createInfo, nullptr, &m_ShaderModule));
		VulkanEngine_DEBUG(fmt::runtime("Created shader module: {}"), m_ShaderPath.string());
	}

}