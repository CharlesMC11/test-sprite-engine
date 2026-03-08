#include "atlas.hpp"
#include "memory_map.hpp"

extern void render(const sc::sprite* sprite);

void debug_sprite(const sc::memory_map<sc::sprite>& sprite);

int main(const int argc, const char* argv[])
{
    if (argc < 2) [[unlikely]] {
        perror("Not enough args.");
        return 1;
    }

    const sc::memory_map<sc::atlas> sprite{argv[1]};
    if (!sprite) [[unlikely]] {
        perror("Could not load sprites");
        return 1;
    }

    printf("Count: %llu, Size: %zu", sprite->size(), sprite.size());

    // debug_sprite(sprite);

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
