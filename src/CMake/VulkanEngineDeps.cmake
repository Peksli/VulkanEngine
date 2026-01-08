cmake_minimum_required(VERSION 3.27)
include(FetchContent)

# enable verbose output for FetchContent
set(FETCHCONTENT_QUIET OFF)


# ------------------------------------
# Vulkan library
# ------------------------------------
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	Vulkan::Vulkan
	Vulkan::shaderc_combined
)


# ------------------------------------
# GLFW library via FetchContent
# ------------------------------------
FetchContent_Declare(
  glfw
  URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	glfw
)


# ------------------------------------
# Dear ImGui library via FetchContent
# ------------------------------------
FetchContent_Declare(
  imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.4.tar.gz
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_sources(imgui PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
target_link_libraries(imgui PRIVATE 
	glfw
	Vulkan::Vulkan
)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	imgui
)


# ------------------------------------
# stb_image library via FetchContent
# ------------------------------------
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG f1c79c0
)
FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
set_target_properties(stb PROPERTIES 
	INTERFACE_INCLUDE_DIRECTORIES ${stb_SOURCE_DIR}
)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	stb
)


# ------------------------------------
# glm library via FetchContent
# ------------------------------------
FetchContent_Declare(
  glm
  URL https://github.com/g-truc/glm/archive/refs/tags/1.0.2.tar.gz
)
FetchContent_MakeAvailable(glm)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	glm::glm
)


# ------------------------------------
# spdlog library via FetchContent
# ------------------------------------
FetchContent_Declare(
  spdlog
  URL https://github.com/gabime/spdlog/archive/refs/tags/v1.16.0.tar.gz
)
FetchContent_MakeAvailable(spdlog)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	spdlog
)


# ------------------------------------
# vk-bootstrap library via FetchContent
# ------------------------------------
FetchContent_Declare(
  vkbootstrap
  URL https://github.com/charles-lunarg/vk-bootstrap/archive/refs/tags/v1.4.330.tar.gz
)
FetchContent_MakeAvailable(vkbootstrap)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	vk-bootstrap::vk-bootstrap
)


# ------------------------------------
# google-test library via FetchContent
# ------------------------------------
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
)
FetchContent_MakeAvailable(googletest)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	GTest::gtest_main 
	GTest::gmock_main
)


# ------------------------------------
# vma library via FetchContent
# ------------------------------------
FetchContent_Declare(
	vma
	URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.3.0.tar.gz
)
FetchContent_MakeAvailable(vma)
target_link_libraries(${ENGINE_NAME} PUBLIC 
	GPUOpen::VulkanMemoryAllocator
)


# Finish
function(group_third_party target_name)
    if(TARGET ${target_name})
        set_target_properties(${target_name} PROPERTIES FOLDER "Deps")
    endif()
endfunction()

group_third_party(glfw)
group_third_party(imgui)
group_third_party(stb)
group_third_party(glm)
group_third_party(spdlog)
group_third_party(vk-bootstrap)
group_third_party(gtest)
group_third_party(gtest_main)
group_third_party(gmock)
group_third_party(gmock_main)