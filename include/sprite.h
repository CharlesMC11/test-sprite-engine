#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

#define HEIGHT 32
#define WIDTH 32
#define MAX_PALETTE_SIZE 16

// BGRA
typedef uint16_t Color;

typedef enum : uint8_t {
  // BGRA
  CHAR = 1,   // 5/6/5/0
  FOREST = 3, // 3/10/3/0
  SEA = 5,    // 2/2/12
} Mode;

typedef struct {
  uint8_t index : 3;
  uint8_t glow : 2;
  uint8_t reserved : 3;
} Pixel;

typedef struct {
  char magic[8];
  Mode mode;
  uint8_t anchor_x;
  uint8_t anchor_y;
  uint8_t reserved[21];
  Color palette[MAX_PALETTE_SIZE];
  Pixel pixels[HEIGHT * WIDTH];
} Sprite;

#endif // PIXEL_H
