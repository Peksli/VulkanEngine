cmake_minimum_required(VERSION 3.27)

set(ENGINE_NAME VulkanEngine)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(${ENGINE_NAME})

# File structure
file(GLOB_RECURSE sources 
	"${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/../../EntryPoint/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../EntryPoint/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../EntryPoint/*.comp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../EntryPoint/*.vert"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../EntryPoint/*.frag"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../EntryPoint/*.spv"
)
file(GLOB_RECURSE includes "${CMAKE_CURRENT_SOURCE_DIR}/../*.h")

set(root "${CMAKE_CURRENT_SOURCE_DIR}/../..") # groups relative to
source_group(TREE ${root} FILES ${sources} ${includes})


# Logs
foreach(source ${sources})
    message(STATUS "  - ${source}")
endforeach()


# Configure .exe
add_executable(${ENGINE_NAME}
	${sources} 
	${includes}
)

target_precompile_headers(${ENGINE_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../VulkanEnginePch.h
) 
target_include_directories(${ENGINE_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Configure deps
include(VulkanEngineDeps.cmake)