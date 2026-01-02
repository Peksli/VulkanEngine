#include <VulkanEngine.h>


int main()
{
	VulkanEngine::ApplicationSpecification appSpec;
	appSpec.width = 1280;
	appSpec.height = 720;
	appSpec.windowName = "Vulkan Engine";

	VulkanEngine::Application app(appSpec);

	app.Run();
	
	app.Shutdown();

	return 0;
}