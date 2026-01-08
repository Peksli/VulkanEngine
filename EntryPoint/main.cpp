#include <VulkanEngine.h>
#include "DebugLayer.h"
#include "MainLayer.h"


class Application : public VulkanEngine::Application
{
public:
	Application(VulkanEngine::ApplicationSpecification& spec)
		: VulkanEngine::Application(spec)
	{
		Client_INFO("Creating application!"); 
		PushLayer(std::make_shared<MainLayer>("Main layer"));
		PushOverlay(std::make_shared<DebugLayer>("Debug overlay"));
	}

	virtual ~Application()
	{
		Client_INFO("Destroying application!");
	}
};


int main()
{
	VulkanEngine::ApplicationSpecification appSpec;
	appSpec.windowWidth   = 1280;
	appSpec.windowHeight  = 720;
	appSpec.windowName    = "Vulkan Engine";

	Application app(appSpec);

	app.Run();
	
	app.Shutdown();

	return 0;
}