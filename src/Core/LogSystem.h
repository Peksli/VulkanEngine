#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>


namespace VulkanEngine {

	using sink = spdlog::sinks::stdout_color_sink_mt;
	using logger = spdlog::logger;

	class LogSystem
	{
	public:
		static std::shared_ptr<sink> vulkanEngineSink;
		static std::shared_ptr<sink> clientSink;

		static std::shared_ptr<logger> vulkanEngineLogger;
		static std::shared_ptr<logger> clientLogger;

		static void Initialize();
		static std::shared_ptr<logger> GetVulkanEngineLogger() { return vulkanEngineLogger; }
		static std::shared_ptr<logger> GetClientLogger() { return clientLogger; }

		template<typename... Args> static void VulkanEngineTrace(Args&&... args) {    vulkanEngineLogger->trace(std::forward<Args>(args)...); }
		template<typename... Args> static void VulkanEngineDebug(Args&&... args) {    vulkanEngineLogger->debug(std::forward<Args>(args)...); }
		template<typename... Args> static void VulkanEngineInfo(Args&&... args) {     vulkanEngineLogger->info(std::forward<Args>(args)...); }
		template<typename... Args> static void VulkanEngineWarn(Args&&... args) {     vulkanEngineLogger->warn(std::forward<Args>(args)...); }
		template<typename... Args> static void VulkanEngineError(Args&&... args) {    vulkanEngineLogger->error(std::forward<Args>(args)...); }
		template<typename... Args> static void VulkanEngineCritical(Args&&... args) { vulkanEngineLogger->critical(std::forward<Args>(args)...); }

		template<typename... Args> static void ClientTrace(Args&&... args) {	clientLogger->trace(std::forward<Args>(args)...); }
		template<typename... Args> static void ClientDebug(Args&&... args) {	clientLogger->debug(std::forward<Args>(args)...); }
		template<typename... Args> static void ClientInfo(Args&&... args) {		clientLogger->info(std::forward<Args>(args)...); }
		template<typename... Args> static void ClientWarn(Args&&... args) {		clientLogger->warn(std::forward<Args>(args)...); }
		template<typename... Args> static void ClientError(Args&&... args) {	clientLogger->error(std::forward<Args>(args)...); }
		template<typename... Args> static void ClientCritical(Args&&... args) { clientLogger->critical(std::forward<Args>(args)...); }
	};

}


#define VulkanEngine_TRACE(...)			  VulkanEngine::LogSystem::VulkanEngineTrace(__VA_ARGS__)
#define VulkanEngine_DEBUG(...)			  VulkanEngine::LogSystem::VulkanEngineDebug(__VA_ARGS__)
#define VulkanEngine_INFO(...)			  VulkanEngine::LogSystem::VulkanEngineInfo(__VA_ARGS__)
#define VulkanEngine_WARN(...)			  VulkanEngine::LogSystem::VulkanEngineWarn(__VA_ARGS__)
#define VulkanEngine_ERROR(...)			  VulkanEngine::LogSystem::VulkanEngineError(__VA_ARGS__)
#define VulkanEngine_CRITICAL(...)		  VulkanEngine::LogSystem::VulkanEngineCritical(__VA_ARGS__)

#define Client_TRACE(...)				  VulkanEngine::LogSystem::ClientTrace(__VA_ARGS__)
#define Client_DEBUG(...)				  VulkanEngine::LogSystem::ClientDebug(__VA_ARGS__)
#define Client_INFO(...)				  VulkanEngine::LogSystem::ClientInfo(__VA_ARGS__)
#define Client_WARN(...)				  VulkanEngine::LogSystem::ClientWarn(__VA_ARGS__)
#define Client_CRITICAL(...)			  VulkanEngine::LogSystem::ClientCritical(__VA_ARGS__)
