cmake_minimum_required(VERSION 3.24)

project(vulkan_testing)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED
        OPTIONAL_COMPONENTS
        shaderc_combined)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/glfw-3.3.8)
add_subdirectory(lib/glm-0.9.9.8)

set(LIB_SOURCE_FILES
        lib/whereami-ba364cd/src/whereami.c)

set(SOURCE_FILES
        src/main.cpp
        src/app.hpp src/app.cpp
        src/logger.hpp src/logger.cpp
        src/renderer.hpp src/renderer.cpp
        src/window.hpp src/window.cpp)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${LIB_SOURCE_FILES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        glfw
        glm)

target_include_directories(${PROJECT_NAME} PRIVATE
        lib/spdlog-1.11.0/include
        lib/vma-3.0.1/include
        lib/whereami-ba364cd/include)