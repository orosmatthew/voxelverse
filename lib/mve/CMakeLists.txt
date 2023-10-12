cmake_minimum_required(VERSION 3.24)

project(mve)
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw-3.3.8)

add_library(mve STATIC
        external/spirv-reflect-1.3.250.0/src/spirv_reflect.cpp
        src/renderer.cpp
        src/window.cpp
        src/vertex_data.cpp
        src/shader.cpp
        src/monitor.cpp
        src/logger.cpp)

target_link_libraries(mve Vulkan::Vulkan glfw)

target_include_directories(mve PUBLIC
        external/spirv-reflect-1.3.250.0/include
        external/vma-3.0.1/include
        external/stb-8b5f1f3/include
        include)

target_include_directories(mve PRIVATE external/spdlog-1.11.0/include)