#include "sprite.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

#define SPRITE_SIZE_BYTES sizeof(Sprite)

extern void render(Sprite* sprite);

void debug_sprite(const Sprite* sprite);

int main(const int argc, const char* argv[])
{
    if (argc < 2)
        return 1;

    const auto fd = open(argv[1], O_RDONLY);
    if (fd < 0)
        return 1;

    const auto sprite = (Sprite*) mmap(
            nullptr, SPRITE_SIZE_BYTES, PROT_READ, MAP_SHARED, fd, 0);
    if (sprite == MAP_FAILED)
        return 1;

    debug_sprite(sprite);

    render(sprite);

    munmap(sprite, SPRITE_SIZE_BYTES);
    return 0;
}

void debug_sprite(const Sprite* sprite)
{
    printf("Encoding: %d\n",
            sprite->encoding); //
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            const uint_fast8_t i = sprite->pixels[y * HEIGHT + x].index;

            printf("%c", i > 0 ? '#' : ' ');
        }
        puts("");
    }
}
