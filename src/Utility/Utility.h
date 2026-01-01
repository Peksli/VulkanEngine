#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <filesystem>
#include <string>
#include <memory>

#include "Core/LogSystem.h"


#define CHECK_VK_RES(res) \
    if(res != VK_SUCCESS) { \
        VulkanEngine_CRITICAL(fmt::runtime("Vulkan Validation Error: {0}"), string_VkResult(res)); \
        abort(); \
    }