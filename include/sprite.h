#pragma once

#ifdef __cplusplus
#include <cstdint>

namespace sc {
    extern "C" {
#else
#include <stdalign.h>
#include <stdint.h>
#endif

#define WIDTH 32
#define HEIGHT 32
#define MAX_PALETTE_SIZE 16

    typedef enum color_encoding : uint8_t {
        DEFAULT = 1, // R5G6B5
        WARM, // R6G5B5
        COOL, // R5G5B6
    } color_encoding;

    typedef uint16_t color;

    typedef struct pixel {
        uint8_t index : 4;
        uint8_t alpha : 2;
        uint8_t glow : 1;
        uint8_t reserved : 1;
    } pixel;

    typedef struct __attribute__((aligned(16))) sprite {
        uint8_t min_x, min_y;
        uint8_t max_x, max_y;
        uint8_t anchor_x, anchor_y;
        color_encoding encoding;
        uint8_t reserved;
        color palette[MAX_PALETTE_SIZE];
        pixel pixels[WIDTH * HEIGHT];
        uint64_t padding;
    } sprite;

#ifdef __cplusplus
    } // extern "C"
} // namespace sc
#endif
