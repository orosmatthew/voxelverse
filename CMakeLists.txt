cmake_minimum_required(VERSION 3.20)

project(vulkan_testing)

set(CMAKE_CXX_STANDARD 20)

set(SHADERC_SKIP_TESTS ON)

find_package(Vulkan REQUIRED)

add_subdirectory(lib/spdlog-1.10.0)
add_subdirectory(lib/glfw-3.3.8)
add_subdirectory(lib/Vulkan-Headers-1.3.230)
add_subdirectory(lib/glslang-sdk-1.3.224.1)
add_subdirectory(lib/SPIRV-Tools-2022.2)
add_subdirectory(lib/shaderc-2022.2)
add_subdirectory(lib/glm-0.9.9.8)
add_subdirectory(lib/VulkanMemoryAllocator-3.0.1)

set(source_files
        src/main.cpp
        src/mve.hpp src/mve.cpp
        src/app.hpp src/app.cpp
        src/logger.hpp src/logger.cpp
        src/renderer.hpp src/renderer.cpp
        src/window.hpp src/window.cpp)

add_executable(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME}
        glfw
        Vulkan-Headers
        ${Vulkan_LIBRARIES}
        shaderc
        spdlog
        glm
        VulkanMemoryAllocator
        )
