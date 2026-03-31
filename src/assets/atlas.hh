#ifndef SC_ASSETS_ATLAS_HH
#define SC_ASSETS_ATLAS_HH

#include <cstddef>
#include <cstdint>
#include <span>

#include "assets/palette_index.hh"
#include "assets/sprite.hh"
#include "assets/sprite16_index.hh"
#include "assets/sprite32_index.hh"
#include "core/core.hh"
#include "graphics/graphics_types.hh"

namespace sc::assets {

    static constexpr std::uint64_t kAtlasMagicBytes{0x3476205441204353ULL};

    /**
     * A contiguous collection of sprites.
     *
     * This class is designed to memory-mapped by `sc::core::mapped_view`.
     */
    struct alignas(core::kCacheAlignment) atlas final {
        // Static methods

        [[nodiscard]] static constexpr bool validate(
                const void* ptr, std::size_t mapped_size) noexcept;

        // Delete constructors because the atlas is never constructed.

        atlas() = delete;

        atlas(const atlas&) = delete;
        atlas& operator=(const atlas&) = delete;

        atlas(atlas&&) = delete;
        atlas& operator=(atlas&&) = delete;

        ~atlas() = default;

        // Operators

        [[nodiscard]] constexpr auto operator[](palette_index i) const noexcept
                -> const graphics::palette&;

        [[nodiscard]] constexpr auto operator[](sprite16_index i) const noexcept
                -> const sprite16&;

        [[nodiscard]] constexpr auto operator[](sprite32_index i) const noexcept
                -> const sprite32&;

        // Accessors

        [[nodiscard]] constexpr auto palette_span() const noexcept
                -> std::span<const graphics::palette>;

        [[nodiscard]] constexpr auto sprite16_span() const noexcept
                -> std::span<const sprite16>;

        [[nodiscard]] constexpr auto sprite32_span() const noexcept
                -> std::span<const sprite32>;

        // Attributes

        struct alignas(core::kNeonAlignment) metadata final {
            const std::uint64_t magic;
            const std::uint32_t sprite16_count;
            const std::uint16_t sprite32_count;
            const std::uint16_t palette_count;
        } meta;

    private:
        [[nodiscard]] constexpr auto data() const noexcept
                -> const std::byte* __restrict;
    };

    // Static methods

    [[nodiscard]] constexpr bool atlas::validate(
            const void* ptr, const std::size_t mapped_size) noexcept
    {
        if (!ptr || mapped_size < sizeof(atlas))
            return false;

        const auto [magic, sprite16_count, sprite32_count, palette_count]{
                static_cast<const atlas*>(ptr)->meta};
        if (magic != kAtlasMagicBytes)
            return false;

        const std::size_t expected_size{sizeof(metadata) +
                sizeof(graphics::palette) * palette_count +
                sizeof(sprite16) * sprite16_count +
                sizeof(sprite32) * sprite32_count};
        return mapped_size >= expected_size;
    }

    // Operators

    [[nodiscard]] constexpr auto atlas::operator[](
            const palette_index i) const noexcept -> const graphics::palette&
    {
        return palette_span()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const sprite16_index i) const noexcept -> const sprite16&
    {
        return sprite16_span()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const sprite32_index i) const noexcept -> const sprite32&
    {
        return sprite32_span()[static_cast<std::size_t>(i)];
    }

    // Accessors

    [[nodiscard]] constexpr auto atlas::palette_span() const noexcept
            -> std::span<const graphics::palette>
    {
        return {reinterpret_cast<const graphics::palette*>(data()),
                sizeof(graphics::palette) * meta.palette_count};
    }

    [[nodiscard]] constexpr auto atlas::sprite16_span() const noexcept
            -> std::span<const sprite16>
    {
        return {reinterpret_cast<const sprite16*>(data() +
                        sizeof(graphics::palette) * meta.palette_count),
                sizeof(sprite16) * meta.sprite16_count};
    }

    [[nodiscard]] constexpr auto atlas::sprite32_span() const noexcept
            -> std::span<const sprite32>
    {
        return {reinterpret_cast<const sprite32*>(data() +
                        sizeof(graphics::palette) * meta.palette_count +
                        sizeof(sprite16) * meta.sprite16_count),
                sizeof(sprite32) * meta.sprite32_count};
    }

    // Private helpers

    [[nodiscard]] constexpr auto atlas::data() const noexcept
            -> const std::byte* __restrict
    {
        return reinterpret_cast<const std::byte*>(&meta + 1UZ);
    }

} // namespace sc::assets

#endif // SC_ASSETS_ATLAS_HH
