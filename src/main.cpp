#include "atlas.hpp"
#include "memory_map.hpp"

extern void render(const sc::sprite* sprite);

void debug_sprite(const sc::sprite& sprite);

int main(const int argc, const char* argv[])
{
    if (argc < 2) [[unlikely]] {
        perror("Not enough args.");
        return 1;
    }

    const sc::memory_map<sc::atlas> atlas{argv[1]};
    if (!atlas) [[unlikely]] {
        perror("Could not load sprites");
        return 1;
    }

    printf("Count: %llu, Size: %zu\n", atlas->size(), atlas.size());

    for (std::size_t i{0}; i < atlas->size(); ++i) {
        debug_sprite((*atlas)[i]);
    }

    // render(loader.data());
}

void debug_sprite(const sc::sprite& sprite)
{
    printf("Encoding: %u\n", static_cast<std::uint8_t>(sprite.encoding));
    for (uint_fast8_t y = 0; y < sc::HEIGHT; ++y) {
        for (uint_fast8_t x = 0; x < sc::WIDTH; ++x) {
            const uint_fast8_t i = sprite.pixels[y * sc::WIDTH + x].index;

            printf("%c", i > 0 ? '#' : ' ');
        }
        puts("");
    }
}
