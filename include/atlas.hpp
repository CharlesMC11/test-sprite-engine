/**
 * @file atlas.hpp
 * @brief
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "atlas_index.hpp"
#include "sprite.hpp"

namespace sc {

    /**
     * @class atlas
     * @brief A contiguous collection of sprites.
     *
     * This class is designed to live within an `sc::memory_map`. It uses a
     * flexible array member (`data_`) to provide indexed access to sprites
     * loaded directly from an `.atlas` file.
     */
    class alignas(16) atlas final {
    public:
        // Delete constructors because the atlas is mapped, not instantiated.
        atlas() = delete;
        ~atlas() = delete;
        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;
        atlas& operator=(const atlas&) = delete;
        atlas& operator=(atlas&&) = delete;

        [[nodiscard]] constexpr std::uint64_t size() const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                std::size_t i) const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                atlas_index i) const noexcept;

        [[nodiscard]] constexpr bool is_valid(
                std::size_t mapped_size) const noexcept;

    private:
        static constexpr std::size_t EXPECTED_MAGIC_SIZE{8};

        static constexpr char EXPECTED_MAGIC[EXPECTED_MAGIC_SIZE]{
                'S', 'C', ' ', 'A', 'T', 'L', 'A', 'S'};

        char magic_[EXPECTED_MAGIC_SIZE];
        uint64_t count_;
        sprite data_[];
    };

    [[nodiscard]] constexpr std::uint64_t atlas::size() const noexcept
    {
        return count_;
    }

    [[nodiscard]] constexpr const sprite& atlas::operator[](
            const std::size_t i) const noexcept
    {
        return data_[i];
    }

    [[nodiscard]] constexpr const sprite& atlas::operator[](
            const atlas_index i) const noexcept
    {
        return (*this)[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr bool atlas::is_valid(
            const std::size_t mapped_size) const noexcept
    {
        return std::memcmp(magic_, EXPECTED_MAGIC, EXPECTED_MAGIC_SIZE) == 0 &&
                sizeof(atlas) + count_ * sizeof(sprite) == mapped_size;
    }

} // namespace sc
