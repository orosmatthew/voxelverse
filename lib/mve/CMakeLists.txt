cmake_minimum_required(VERSION 3.5)

project(mve VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw-3.4)

add_library(mve STATIC
        external/stb-image-2.30/src/stb_image.c
        external/vma-3.1.0/src/vk_mem_alloc.cpp
        external/spirv-reflect-1.3.290/src/spirv_reflect.c
        external/volk-1.3.270/src/volk.c
        src/logger.cpp
        src/monitor.cpp
        src/renderer.cpp
        src/shader.cpp
        src/window.cpp)

target_link_libraries(mve glfw)

target_include_directories(mve PUBLIC include
        external/nnm-0.3.0/include
        external/spirv-reflect-1.3.290/include
        external/stb-image-2.30/include
        external/spdlog-1.14.1/include
        external/volk-1.3.270/include)
target_include_directories(mve SYSTEM PUBLIC
        external/vulkan-headers-1.3.290/include
        external/vma-3.1.0/include)

if (MVE_BUILD_EXAMPLES)

    if (WIN32)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -stdlib=libc++ -lc++abi")
        endif ()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
        endif ()
    endif ()

    add_executable(hello_triangle examples/hello_triangle/main.cpp)
    target_link_libraries(hello_triangle PRIVATE mve)

    add_executable(rotating_textured_cube examples/rotating_textured_cube/main.cpp)
    target_link_libraries(rotating_textured_cube PRIVATE mve)

    add_executable(first_person examples/first_person/main.cpp)
    target_link_libraries(first_person PRIVATE mve)

    add_executable(model
            examples/external/stb_image_write-1.16/src/stb_image_write.c
            examples/external/tinygltf-2.9.3/src/tiny_gltf.cc
            examples/model/main.cpp)
    target_include_directories(model PRIVATE
            examples/external/stb_image_write-1.16/include
            examples/external/json-3.11.3/include
            examples/external/tinygltf-2.9.3/include)
    target_link_libraries(model PRIVATE mve)

    add_executable(post_processing examples/post_processing/main.cpp)
    target_link_libraries(post_processing PRIVATE mve)

    function(target_add_shaders TARGET OUTPUT_PATH)
        find_program(GLSLANG glslang)
        if (NOT GLSLANG)
            message(FATAL_ERROR "glslang not found")
        endif ()
        file(MAKE_DIRECTORY ${OUTPUT_PATH})
        foreach (SHADER ${ARGN})
            get_filename_component(SHADER_NAME ${SHADER} NAME)
            set(OUTPUT_FILE "${OUTPUT_PATH}/${SHADER_NAME}.spv")
            add_custom_command(
                    OUTPUT ${OUTPUT_FILE}
                    COMMAND ${GLSLANG} -V ${SHADER} -o ${OUTPUT_FILE}
                    DEPENDS ${SHADER}
                    COMMENT "Compiling ${SHADER} to ${OUTPUT_FILE}"
                    VERBATIM
            )
            set_source_files_properties(${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
            target_sources(${TARGET} PRIVATE ${OUTPUT_FILE})
        endforeach ()
    endfunction()

    target_add_shaders(hello_triangle ${CMAKE_SOURCE_DIR}/examples/shaders/bin
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple.vert
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple.frag)

    target_add_shaders(rotating_textured_cube ${CMAKE_SOURCE_DIR}/examples/shaders/bin
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d_textured.vert
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d_textured.frag)

    target_add_shaders(first_person ${CMAKE_SOURCE_DIR}/examples/shaders/bin
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d_textured.vert
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d_textured.frag)

    target_add_shaders(model ${CMAKE_SOURCE_DIR}/examples/shaders/bin
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d.vert
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d.frag)

    target_add_shaders(post_processing ${CMAKE_SOURCE_DIR}/examples/shaders/bin
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d_textured.vert
            ${CMAKE_SOURCE_DIR}/examples/shaders/simple_3d_textured.frag
            ${CMAKE_SOURCE_DIR}/examples/shaders/post_process.vert
            ${CMAKE_SOURCE_DIR}/examples/shaders/post_process.frag)
endif ()
