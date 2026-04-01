cmake_minimum_required(VERSION 4.1)
project(test-sprite-engine C CXX OBJCXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/metal-cpp")

find_library(APPKIT_LIB AppKit REQUIRED)
find_library(METAL_LIB Metal REQUIRED)
find_library(METALKIT_LIB MetalKit REQUIRED)
find_library(QUARTZ_LIB QuartzCore REQUIRED)

add_executable(test-sprite-engine
        "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/metal_bridge.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/physics/physics.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/registry/entity_registry.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/SCAppDelegate.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/SCStage.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/main.mm"
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

target_link_libraries(test-sprite-engine
        ${APPKIT_LIB}
        ${METAL_LIB}
        ${METALKIT_LIB}
        ${QUARTZ_LIB}
)

add_custom_command(OUTPUT shader.metallib
        COMMAND xcrun -sdk macosx metal -c -I "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/shader.metal" -o shader.air
        COMMAND xcrun -sdk macosx metallib shader.air -o "${CMAKE_CURRENT_SOURCE_DIR}/assets/shader.metallib"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/shader.metal"
)

add_custom_target(shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib")
add_dependencies(test-sprite-engine shaders)