/**
 * @file sprite_bank.hpp
 * @brief
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "definitions.hpp"
#include "entity_id.hpp"
#include "sprite.hpp"

namespace sc {

    /**
     * @class sprite_bank
     * @brief A contiguous collection of sprites.
     *
     * This class is designed to live within an `sc::memory_map`. It uses a
     * flexible array member (`data_`) to provide indexed access to sprites
     * loaded directly from an `.atlas` file.
     */
    class alignas(sys::ALIGNMENT) sprite_bank final {
    public:
        // Delete constructors because the sprite_bank is mapped, not
        // instantiated.
        sprite_bank() = delete;
        ~sprite_bank() = delete;
        sprite_bank(const sprite_bank&) = delete;
        sprite_bank(sprite_bank&&) = delete;
        sprite_bank& operator=(const sprite_bank&) = delete;
        sprite_bank& operator=(sprite_bank&&) = delete;

        [[nodiscard]] constexpr std::uint64_t size() const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                std::size_t i) const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                entity_id i) const noexcept;

        [[nodiscard]] constexpr bool is_valid(
                std::size_t mapped_size) const noexcept;

    private:
        static constexpr std::size_t EXPECTED_MAGIC_SIZE{8};

        // Should this be std::array?
        static constexpr char EXPECTED_MAGIC[EXPECTED_MAGIC_SIZE]{
                'S', 'C', ' ', 'A', 'T', 'L', 'A', 'S'};

        char magic_[EXPECTED_MAGIC_SIZE];
        uint64_t count_;
        sprite data_[];
    };

    [[nodiscard]] constexpr std::uint64_t sprite_bank::size() const noexcept
    {
        return count_;
    }

    [[nodiscard]] constexpr const sprite& sprite_bank::operator[](
            const std::size_t i) const noexcept
    {
        return data_[i];
    }

    [[nodiscard]] constexpr const sprite& sprite_bank::operator[](
            const entity_id i) const noexcept
    {
        return (*this)[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr bool sprite_bank::is_valid(
            const std::size_t mapped_size) const noexcept
    {
        // Maybe look into std::ranges?
        return std::memcmp(magic_, EXPECTED_MAGIC, EXPECTED_MAGIC_SIZE) == 0 &&
                sizeof(sprite_bank) + count_ * sizeof(sprite) == mapped_size;
    }

} // namespace sc
