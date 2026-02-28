#include "sprite.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

void debug_sprite(Sprite *sprite);

int main(const int argc, const char *argv[]) {
  if (argc < 2)
    return 1;

  const int fd = open(argv[1], O_RDONLY);
  if (fd < 0)
    return 1;

  Sprite *sprite = mmap(NULL, sizeof(Sprite), PROT_READ, MAP_SHARED, fd, 0);
  if (sprite != MAP_FAILED) {
    debug_sprite(sprite);
    munmap(sprite, sizeof(Sprite));
  }

  close(fd);
  return 0;
}

void debug_sprite(Sprite *sprite) {
  printf("Magic: %8s\n", sprite->magic);
  printf("Mode: %d\n", sprite->mode);

  for (size_t i = 0; i < MAX_PALETTE_SIZE; ++i) {
    printf("Palette[%2lu]: 0x%04X\n", i, sprite->palette[i]);
  }

  puts("Preview");
  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      const uint8_t i = sprite->pixels[y * WIDTH + x].index;
      printf("%c", i > 0 ? '#' : '.');
    }
    puts("");
  }
}
