#include <format>
#include <iostream>

#include "assets/atlas.hh"
#include "assets/sprite.hh"
#include "core/mapped_view.hh"

void debug_sprite(const sc::sprites::sprite32& sprite);

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
              << "\tSprite16 Count: " << view->meta.sprite16_count
              << "\tSprite32 Count: " << view->meta.sprite32_count << std::endl;

    std::size_t i{0u};
    for (; i < view->meta.palette_count; ++i) {
        std::cout << std::format("Palette {:02}: ", i + 1u);
        for (const auto& p: view->palette_span()[i]) {
            std::cout << std::format("{:04X} ", p);
        }
        std::cout << "\n\n";
    }

    for (i = 0u; i < view->meta.sprite32_count; ++i) {
        std::cout << std::format("Sprite {:02}\t", i + 1u);
        debug_sprite(view->sprite32_span()[i]);
    }
}

void debug_sprite(const sc::sprites::sprite32& sprite)
{
    const sc::sprites::metadata& meta{sprite.meta};

    std::cout << std::format(
            "BBox: ({:02}, {:02}, {:02}, {:02})\tOrigin: ({:02.2f}, "
            "{:02.2f})\tEncoding: {}\tPalette: {}\tPhysics: {}\n",
            meta.bbox.min_u, meta.bbox.min_v, meta.bbox.max_u, meta.bbox.max_v,
            meta.origin_u, meta.origin_v,
            static_cast<unsigned>(meta.color_encoding), meta.palette_index,
            meta.physics_type);

    for (uint_fast8_t y = 0; y < sc::sprites::kHeight; ++y) {
        for (uint_fast8_t x = 0; x < sc::sprites::kWidth; ++x) {
            const sc::sprites::packed_pixel pixel = sprite.pixels[y][x];

            const bool a{(pixel & sc::sprites::kMaskAlpha) > 0x00};
            const auto i{pixel & sc::sprites::kMaskPaletteIndex};

            std::cout << (a ? std::format("{:X}*", i) : "  ");
        }
        std::cout << std::endl;
    }
}
