cmake_minimum_required(VERSION 4.1)
project(test-sprite-engine CXX OBJCXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        src/
        vendor/metal-cpp
)

find_package(Python 3.14 COMPONENTS Interpreter Development.Module REQUIRED)
execute_process(
        COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
        OUTPUT_VARIABLE nanobind_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(nanobind CONFIG REQUIRED)

find_library(APPKIT_LIB AppKit REQUIRED)
find_library(METAL_LIB Metal REQUIRED)
find_library(METALKIT_LIB MetalKit REQUIRED)
find_library(QUARTZ_LIB QuartzCore REQUIRED)

add_library(shared INTERFACE
        src/assets/atlas.hh
        src/assets/sprite.hh
        src/core/core.hh
        src/graphics/graphics_types.hh
        src/math/bbox.hh
        src/math/utils.hh
        src/simulation/physics_types.hh
)

nanobind_add_module(_pipeline
        pipeline/library.cc
)
target_link_libraries(_pipeline PRIVATE shared)

add_executable(test-sprite-engine
        src/assets/asset_constants.hh
        src/assets/asset_ids.hh
        src/assets/atlas.cc
        src/assets/sprite.cc
        src/core/input.hh
        src/core/mapped_view.hh
        src/core/memory.hh
        src/graphics/display_constants.hh
        src/graphics/graphics_types.cc
        src/graphics/metal_bridge.hh
        src/graphics/metal_bridge.cc
        src/graphics/render_constants.hh
        src/graphics/shader.metal
        src/math/bbox.hh
        src/platform/SCAppDelegate.hh
        src/platform/SCAppDelegate.mm
        src/platform/SCStage.hh
        src/platform/SCStage.mm
        src/platform/main.mm
        src/registry/entity_registry.hh
        src/registry/entity_registry.cc
        src/simulation/physics.hh
        src/simulation/physics.cc
        src/simulation/spatial_grid.hh
        src/simulation/spatial_grid.cc
        src/simulation/world.hh
        src/simulation/world.cc
)
target_link_libraries(test-sprite-engine PRIVATE
        shared
        ${APPKIT_LIB}
        ${METAL_LIB}
        ${METALKIT_LIB}
        ${QUARTZ_LIB}
)

target_compile_options(test-sprite-engine PRIVATE
        -arch arm64
        -march=native
        -falign-functions=16
        -fcolor-diagnostics
        -Wall -Wextra -Wpedantic
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test-sprite-engine PRIVATE -O0 -g -fsanitize=address,undefined)
    target_link_options(test-sprite-engine PRIVATE -fsanitize=address,undefined)
else ()
    target_compile_options(test-sprite-engine PRIVATE -O3 -flto)
    target_link_options(test-sprite-engine PRIVATE -flto -Wl,-dead_strip)
endif ()

add_custom_command(OUTPUT shader.metallib
        COMMAND xcrun -sdk macosx metal -c -I "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/shader.metal" -o shader.air
        COMMAND xcrun -sdk macosx metallib shader.air -o "${CMAKE_CURRENT_SOURCE_DIR}/assets/shader.metallib"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/shader.metal"
)

add_custom_target(shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib")
add_dependencies(test-sprite-engine shaders)
