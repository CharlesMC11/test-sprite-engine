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

    enum ColorEncoding : uint8_t {
        DEFAULT = 1, // R5G6B5
        WARM, // R6G5B5
        COOL, // R5G5B6
    };

    typedef uint16_t Color;

    struct Pixel {
        uint8_t index : 4;
        uint8_t alpha : 2;
        uint8_t glow : 1;
        uint8_t reserved : 1;
    };

    struct alignas(32) Sprite {
        char magic[8];
        char name[16];
        ColorEncoding encoding;
        uint8_t anchor_x;
        uint8_t anchor_y;
        uint8_t reserved[5];
        Color palette[MAX_PALETTE_SIZE];
        Pixel pixels[WIDTH * HEIGHT];
    };

#ifdef __cplusplus
    } // extern "C"
} // namespace sc
#endif

#endif // SPRITE_H
