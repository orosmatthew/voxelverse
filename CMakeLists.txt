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
        src/app.cpp
        src/logger.cpp
        src/renderer.cpp
        src/window.cpp
        src/vertex_data.cpp)

set(LIBS
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        glfw
        glm)

set(LIB_INCLUDES
        lib/spdlog-1.11.0/include
        lib/vma-3.0.1/include
        lib/whereami-ba364cd/include)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        ${LIB_SOURCE_FILES}
        ${SOURCE_FILES}
        src/main.cpp)

target_link_libraries(${PROJECT_NAME} ${LIBS})

target_include_directories(${PROJECT_NAME} PRIVATE ${LIB_INCLUDES})

set(TEST_LIB_SOURCE_FILES
        lib/catch2-3.1.1/src/catch_amalgamated.cpp)

set(TEST_SOURCE_FILES
        tests/vertex_data.cpp)

set(TEST_LIB_INCLUDES
        lib/catch2-3.1.1/include)

add_executable(tests)

target_sources(tests PRIVATE
        ${LIB_SOURCE_FILES}
        ${SOURCE_FILES}
        ${TEST_LIB_SOURCE_FILES}
        ${TEST_SOURCE_FILES})

target_link_libraries(tests ${LIBS})

target_include_directories(tests PRIVATE
        ${LIB_INCLUDES}
        ${TEST_LIB_INCLUDES})
