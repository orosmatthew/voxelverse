cmake_minimum_required(VERSION 3.24)

project(vulkan_testing)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED
        OPTIONAL_COMPONENTS
        shaderc_combined)

add_subdirectory(lib/glfw-3.3.8)
add_subdirectory(lib/glm-0.9.9.8)

set(source_files
        src/main.cpp
        src/mve.hpp src/mve.cpp
        src/app.hpp src/app.cpp
        src/logger.hpp src/logger.cpp
        src/renderer.hpp src/renderer.cpp
        src/window.hpp src/window.cpp)

add_executable(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME}
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        glfw
        glm)

target_include_directories(${PROJECT_NAME} PRIVATE
        lib/spdlog-1.10.0/include)