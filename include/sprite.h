#ifndef SPRITE_H
#define SPRITE_H

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

    typedef enum ColorEncoding : uint8_t {
        DEFAULT = 1, // R5G6B5
        WARM, // R6G5B5
        COOL, // R5G5B6
    } ColorEncoding;

    typedef uint16_t Color;

    typedef struct Pixel {
        uint8_t index : 4;
        uint8_t alpha : 2;
        uint8_t glow : 1;
        uint8_t reserved : 1;
    } Pixel;

    typedef struct __attribute__((aligned(16))) Sprite {
        uint8_t min_x, min_y;
        uint8_t max_x, max_y;
        uint8_t anchor_x, anchor_y;
        ColorEncoding encoding;
        uint8_t reserved;
        Color palette[MAX_PALETTE_SIZE];
        Pixel pixels[WIDTH * HEIGHT];
        uint64_t padding;
    } Sprite;

#ifdef __cplusplus
    } // extern "C"
} // namespace sc
#endif

#endif // SPRITE_H
