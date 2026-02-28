cmake_minimum_required(VERSION 4.1)
project(pixel_tester C CXX ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64 -march=native
    -falign-functions=16 -fcolor-diagnostics -Wall -Wextra -Wpedantic")

set(CMAKE_CXX_STANDARD 26)

include_directories(include)

find_library(METAL_LIB Metal REQUIRED)
find_library(QUARTZ_LIB QuartzCore REQUIRED)
find_library(APPKIT_LIB AppKit REQUIRED)


add_executable(pixel_tester
        include/pixel.h
        src/engine.s
        src/shader.metal
        tests/test_baker.c)

target_link_libraries(pixel_tester
        ${METAL_LIB}
        ${QUARTZ_LIB}
        ${APPKIT_LIB}
)

add_custom_command(OUTPUT shader.metallib
        COMMAND xcrun -sdk macosx metal -c src/shader.metal -o shader.air
        COMMAND xcrun -sdk macosx metallib shader.air -o shader.metallib
        DEPENDS src/shader.metal)