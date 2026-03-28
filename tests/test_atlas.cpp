#include <cstdio>
#include <iostream>

#include "../include/atlas.hh"
#include "../include/mapped_view.hh"
#include "../include/sprite.hh"

void debug_sprite(const sc::sprites::sprite32x32& sprite);

int main(const int argc, const char* argv[])
{
    if (argc < 2) [[unlikely]] {
        std::cerr << "ERROR: Must enter path to atlas!\n";
        return 1;
    }

    const sc::core::mapped_view<sc::sprites::atlas> view{argv[1]};
    if (!view) [[unlikely]] {
        std::cerr << "ERROR: Could not load sprites\n";
        return 1;
    }

    std::cout << "Atlas File Size: " << view.size()
              << " bytes\tPalette Count: " << view->meta.palette_count
              << "\tSprite Count: " << view->meta.sprite_count << std::endl;

    for (std::size_t i{0}; i < view->meta.sprite_count; ++i) {
        std::cout << "Sprite " << i + 1 << "\t";
        debug_sprite(view->sprites()[i]);
    }
}

void debug_sprite(const sc::sprites::sprite32x32& sprite)
{
    const sc::sprites::metadata& meta{sprite.meta};

    std::cout << "Encoding " << static_cast<int>(meta.color_encoding)
              << std::endl;

    for (uint_fast8_t y = 0; y < sc::sprites::kHeight; ++y) {
        for (uint_fast8_t x = 0; x < sc::sprites::kWidth; ++x) {
            const sc::sprites::packed_pixel pixel = sprite.pixels[y][x];

            const bool a{(pixel & sc::sprites::kMaskAlpha) > 0x00};
            const auto i{pixel & sc::sprites::kMaskPaletteIndex};

            if (a)
                std::printf("%X", i);
            else
                std::cout << ' ';
        }
        puts("");
    }
}
