#pragma once

#include <memory>
#include <string>

#include <VulkanEngine.h>


class DebugLayer : public VulkanEngine::Layer
{
public:
	DebugLayer(std::string name);
	virtual ~DebugLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate() override;
	void OnEvent()  override;
};