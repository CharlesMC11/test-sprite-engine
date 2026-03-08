cmake_minimum_required(VERSION 4.1)
project(pixel_tester C CXX ASM OBJCXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MY_FLAGS "-arch arm64 -march=native -falign-functions=16 -fcolor-diagnostics -Wall -Wextra -Wpedantic")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MY_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_FLAGS}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/vendor/metal-cpp")

find_library(APPKIT_LIB AppKit REQUIRED)
find_library(METAL_LIB Metal REQUIRED)
find_library(METALKIT_LIB MetalKit REQUIRED)
find_library(QUARTZ_LIB QuartzCore REQUIRED)

add_executable(pixel_tester
        "${CMAKE_CURRENT_SOURCE_DIR}/include/atlas.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/atlas_index.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/bridge.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/sprite.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/memory_map.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/engine.s"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shader.metal"
)

target_link_libraries(pixel_tester
        ${APPKIT_LIB}
        ${METAL_LIB}
        ${METALKIT_LIB}
        ${QUARTZ_LIB}
)

add_custom_command(OUTPUT shader.metallib
        COMMAND xcrun -sdk macosx metal -c "${CMAKE_CURRENT_SOURCE_DIR}/src/shader.metal" -o shader.air
        COMMAND xcrun -sdk macosx metallib shader.air -o shader.metallib
        DEPENDS src/shader.metal
)

target_sources(pixel_tester PRIVATE shader.metallib)
