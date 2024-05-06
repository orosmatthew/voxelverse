cmake_minimum_required(VERSION 3.24)

project(voxelverse)

set(CMAKE_CXX_STANDARD 20)

function(add_shaders TARGET)
    find_program(GLSLANGVALIDATOR glslangValidator)
    foreach (SHADER ${ARGN})
        set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/src/client/shader/${SHADER})
        set(current-output-path ${CMAKE_CURRENT_SOURCE_DIR}/res/bin/shader/${SHADER}.spv)

        # Add a custom command to compile GLSL to SPIR-V using glslangvalidator.
        get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
        file(MAKE_DIRECTORY ${current-output-dir})

        add_custom_command(
                OUTPUT ${current-output-path}
                COMMAND ${GLSLANGVALIDATOR} -V -o ${current-output-path} ${current-shader-path}
                DEPENDS ${current-shader-path}
                IMPLICIT_DEPENDS CXX ${current-shader-path}
                VERBATIM)

        # Make sure our build depends on this output.
        set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
        target_sources(${TARGET} PRIVATE ${current-output-path})
    endforeach ()
endfunction()

set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)

set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LEVELDB_INSTALL OFF CACHE BOOL "" FORCE)


add_subdirectory(external/freetype-2.13.2 SYSTEM)
add_subdirectory(external/leveldb-1.23 SYSTEM)
add_subdirectory(external/enet-1.3.18 SYSTEM)

set(LIB_SOURCE_FILES
        external/whereami-ba364cd/src/whereami.c
        external/lz4-1.9.4/src/lz4.c
        external/lz4-1.9.4/src/lz4hc.c)

add_subdirectory(lib/mve)

set(SOURCE_FILES
        src/client/app.cpp
        src/common/logger.cpp
        src/client/util.cpp
        src/common/fixed_loop.cpp
        src/client/chunk_mesh.cpp
        src/client/chunk_data.cpp
        src/client/world_generator.cpp
        src/client/world_data.cpp
        src/client/world_renderer.cpp
        src/client/player.cpp
        src/client/ui_pipeline.cpp
        src/client/wire_box_mesh.cpp
        src/client/world.cpp
        src/client/save_file.cpp
        src/client/ui/nine_patch.cpp
        src/client/ui/hotbar.cpp
        src/client/ui/crosshair.cpp
        src/client/text_pipeline.cpp
        src/client/ui/debug_overlay.cpp
        src/client/ui/hud.cpp
        src/client/ui/console.cpp
        src/client/ui/button.cpp
        src/client/ui/pause_menu.cpp
        src/client/chunk_controller.cpp
        src/client/lighting.cpp
        src/client/ui/options_menu.cpp
        src/client/options.cpp
        src/server/server.cpp)

set(LIBS
        enet
        freetype
        leveldb
        mve)
if (WIN32)
    list(APPEND LIBS winmm.lib Ws2_32.lib)
endif ()

set(LIB_INCLUDES
        external/spdlog-1.12.0/include
        external/whereami-ba364cd/include
        external/tinyobjloader-45f683c/include
        external/fast-noise-lite-1.1.0/include
        external/cereal-1.3.2/include
        external/lz4-1.9.4/include
        external/thread-pool-3.5.0/include
        external/nlohmann-json-3.11.3/include
        external/enet-1.3.18/include)

add_executable(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PUBLIC RES_PATH="./res")

if (WIN32)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -stdlib=libc++ -lc++abi")
    endif ()
endif ()

target_sources(${PROJECT_NAME} PRIVATE
        ${LIB_SOURCE_FILES}
        ${SOURCE_FILES}
        src/client/main.cpp)

target_link_libraries(${PROJECT_NAME} ${LIBS})

target_include_directories(${PROJECT_NAME} PRIVATE ${LIB_INCLUDES})

add_shaders(voxelverse
        simple.frag
        simple.vert
        color.frag
        color.vert
        ui.frag
        ui.vert
        text.vert
        text.frag)

#set(TEST_LIB_SOURCE_FILES
#        external/catch2-3.3.2/src/catch_amalgamated.cpp)
#
#set(TEST_SOURCE_FILES
#        tests/vertex_data.cpp)
#
#set(TEST_LIB_INCLUDES
#        external/catch2-3.3.2/include)
#
#add_executable(tests)
#
#target_compile_definitions(tests PUBLIC RES_PATH="../res")
#
#target_sources(tests PRIVATE
#        ${LIB_SOURCE_FILES}
#        ${SOURCE_FILES}
#        ${TEST_LIB_SOURCE_FILES}
#        ${TEST_SOURCE_FILES})
#
#target_link_libraries(tests ${LIBS})
#
#target_include_directories(tests PRIVATE
#        ${LIB_INCLUDES}
#        ${TEST_LIB_INCLUDES})
