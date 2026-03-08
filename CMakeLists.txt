cmake_minimum_required(VERSION 4.1)
project(pixel_tester C CXX ASM OBJCXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MY_FLAGS "-arch arm64 -march=native -falign-functions=16 -fcolor-diagnostics -Wall -Wextra -Wpedantic")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MY_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_FLAGS}")

include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/metal-cpp")

find_library(APPKIT_LIB AppKit REQUIRED)
find_library(METAL_LIB Metal REQUIRED)
find_library(METALKIT_LIB MetalKit REQUIRED)
find_library(QUARTZ_LIB QuartzCore REQUIRED)

add_executable(pixel_tester
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/SCAppDelegate.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/SCStage.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/main.mm"
)

target_link_libraries(pixel_tester
        ${APPKIT_LIB}
        ${METAL_LIB}
        ${METALKIT_LIB}
        ${QUARTZ_LIB}
)

add_custom_command(OUTPUT shader.metallib
        COMMAND xcrun -sdk macosx metal -c -I "${CMAKE_CURRENT_SOURCE_DIR}/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/shader.metal" -o shader.air
        COMMAND xcrun -sdk macosx metallib shader.air -o shader.metallib
        DEPENDS src/shader.metal
)

add_custom_target(shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib")
add_dependencies(pixel_tester shaders)