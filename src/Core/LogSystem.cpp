#include "Core/LogSystem.h"


namespace VulkanEngine {

	std::shared_ptr<sink> LogSystem::vulkanEngineSink;
	std::shared_ptr<sink> LogSystem::clientSink;

	std::shared_ptr<logger> LogSystem::vulkanEngineLogger;
	std::shared_ptr<logger> LogSystem::clientLogger;

	void LogSystem::Initialize()
	{
		vulkanEngineSink = std::make_shared<sink>();
		clientSink = std::make_shared<sink>();

		vulkanEngineSink->set_pattern("%^[%T] %n: %v%$");
		clientSink->set_pattern("[%T] [%l] %n: %v");

		vulkanEngineLogger = std::make_shared<logger>("vulkanLogger", vulkanEngineSink);
		clientLogger = std::make_shared<logger>("clientLogger", clientSink);

		vulkanEngineLogger->set_level(spdlog::level::trace);
		clientLogger->set_level(spdlog::level::trace);
	}

}