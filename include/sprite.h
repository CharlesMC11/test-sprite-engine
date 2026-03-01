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
        DEFAULT = 1, // 5-6-5
        WARM = 2, // 6-5-5
        FOREST = 3, // 3-10-3
        SEA = 5, // 2-2-12
    } ColorMode;

    // BRGA
    typedef uint16_t Color;

    typedef struct {
        uint8_t index : 4;
        uint8_t reserved : 4;
    } Pixel;

    typedef struct alignas(32) Sprite {
        char magic[8];
        ColorMode mode;
        uint8_t anchor_x;
        uint8_t anchor_y;
        Color palette[MAX_PALETTE_SIZE];
        Pixel pixels[WIDTH * HEIGHT];
    } Sprite;

#ifdef __cplusplus
    } // extern "C"
} // namespace sc
#endif

#endif // SPRITE_H
