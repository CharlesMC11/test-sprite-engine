#ifndef SPRITE_H
#define SPRITE_H

#ifdef __cplusplus
#include <cstdint>

namespace sc {
    extern "C" {
#else
#include <stdint.h>
#endif

#define WIDTH 32
#define HEIGHT 32
#define MAX_PALETTE_SIZE 16

    // Color Bit Distribution
    typedef enum ColorMode : uint8_t {
        DEFAULT = 1, // R5G6B5
        WARM, // R6G5B5
        COOL, // R5G5B6
        FOREST, // R3G10B3
        SEA, // RG2B12
    } ColorMode;

    // BRGA
    typedef uint16_t Color;

    typedef struct {
        uint8_t index : 4;
        uint8_t alpha : 2;
        uint8_t reserved : 2;
    } Pixel;

    typedef struct alignas(32) Sprite {
        char magic[8];
        ColorMode mode;
        uint8_t anchor_x;
        uint8_t anchor_y;
        uint8_t reserved[21];
        Color palette[MAX_PALETTE_SIZE];
        Pixel pixels[WIDTH * HEIGHT];
    } Sprite;

#ifdef __cplusplus
    } // extern "C"
} // namespace sc
#endif

#endif // SPRITE_H
