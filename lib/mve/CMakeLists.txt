cmake_minimum_required(VERSION 3.24)

project(mve)
set(CMAKE_CXX_STANDARD 20)

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

target_link_libraries(mve glfw)

target_include_directories(mve PUBLIC external/spirv-reflect-1.3.250.0/include include)