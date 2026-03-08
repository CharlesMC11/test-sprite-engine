#include "memory_map.hpp"
#include "sprites.h"

extern void render(const sc::sprite* sprite);

void debug_sprite(const sc::memory_map<sc::sprite>& sprite);

int main(const int argc, const char* argv[])
{
    if (argc < 2) [[unlikely]]
        return 1;

    const sc::memory_map<sc::sprite> sprite{argv[1]};
    if (!sprite) [[unlikely]]
        return 1;

    debug_sprite(sprite);

    // render(loader.data());
}

void debug_sprite(const sc::memory_map<sc::sprite>& sprite)
{
    printf("Encoding: %d\n", sprite->encoding);
    for (uint_fast8_t y = 0; y < HEIGHT; ++y) {
        for (uint_fast8_t x = 0; x < WIDTH; ++x) {
            const uint_fast8_t i = sprite->pixels[y * WIDTH + x].index;

            printf("%c", i > 0 ? '#' : ' ');
        }
        puts("");
    }
}
