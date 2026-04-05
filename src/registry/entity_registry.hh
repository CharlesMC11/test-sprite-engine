#ifndef SC_REGISTRY_SCENE_REGISTRY_HH
#define SC_REGISTRY_SCENE_REGISTRY_HH

#include <algorithm>
#include <cstddef>
#include <ostream>

#include "assets/asset_ids.hh"
#include "core/core.hh"
#include "core/memory.hh"

/**
 * Register a channel accessor method for the registry.
 *
 * @param EnumType
 * The enum referred to by the buffer.
 *
 * @param DataType
 * The type contained in the buffer.
 *
 * @param enum_val
 * The enum value to access.
 *
 * @param buffer_name
 * The name of the buffer.
 */
#define SC_REGISTER_ACCESSOR(EnumType, DataType, enum_val, buffer_name)        \
    [[nodiscard]] constexpr auto enum_val##_ptr() noexcept                     \
            -> DataType* __restrict                                            \
    {                                                                          \
        return get_ptr<EnumType, DataType, false>(                             \
                buffer_name, EnumType::enum_val);                              \
    }                                                                          \
                                                                               \
    [[nodiscard]] constexpr auto enum_val##_ptr() const noexcept               \
            -> const DataType* __restrict                                      \
    {                                                                          \
        return get_ptr<EnumType, DataType, true>(                              \
                buffer_name, EnumType::enum_val);                              \
    }

#define SC_REGISTER_XFORM_CHANNEL_ACCESSOR(enum_val)                           \
    SC_REGISTER_ACCESSOR(xform_channel, float, enum_val, xform_buffer_)

#define SC_REGISTER_INDEX_CHANNEL_ACCESSOR(enum_val)                           \
    SC_REGISTER_ACCESSOR(index_channel, core::index_t, enum_val, index_buffer_)

/**
 * Register channel accessor method for a buffer_swappable channel.
 *
 * @param enum_val
 * The enum value to access.
 *
 * @param enum_swap
 * The enum value the channel is swappable with.
 */
#define SC_REGISTER_SWAPPABLE_ACCESSOR(enum_val, enum_swap)                    \
    [[nodiscard]] constexpr auto enum_val##_ptr() noexcept                     \
            -> float* __restrict                                               \
    {                                                                          \
        return get_ptr<xform_channel, float, false>(xform_buffer_,             \
                channels_swapped_ ? xform_channel::enum_val                    \
                                  : xform_channel::enum_swap);                 \
    }                                                                          \
                                                                               \
    [[nodiscard]] constexpr auto enum_val##_ptr() const noexcept               \
            -> const float* __restrict                                         \
    {                                                                          \
        return get_ptr<xform_channel, float, true>(xform_buffer_,              \
                channels_swapped_ ? xform_channel::enum_val                    \
                                  : xform_channel::enum_swap);                 \
    }

namespace sc {

    /**
     * @struct entity_registry
     * @brief
     */
    class entity_registry final {
    public:
        // Enums

        enum class xform_channel : std::uint8_t {
            x_pos,
            y_pos,
            z_pos,
            x_vel,
            y_vel,
            z_vel,
            new_x_pos,
            new_y_pos,
            new_z_pos,
            count
        };

        enum class index_channel : std::uint8_t {
            sprite_index,
            draw_order,
            next_in_cell,
            count
        };

        // Constructors

        [[nodiscard]] explicit constexpr entity_registry(MTL::Device* device);

        entity_registry(const entity_registry&) = delete;
        entity_registry& operator=(const entity_registry&) = delete;

        entity_registry(entity_registry&&) = delete;
        entity_registry& operator=(entity_registry&&) = delete;

        ~entity_registry() = default;

        // Public methods

        /**
         * Update the current layout.
         *
         * @param dt
         * The delta time.
         */
        void update(float dt) noexcept;

        /**
         * Commit the changes to the registry.
         */
        void commit() noexcept;

        /**
         * Sort the draw order based on screen coordinates.
         */
        void sort_draw() noexcept;

        // Mutators

        /**
         * Reserve space for a number of entities in the registry.
         *
         * @param n
         * The number of entities to reserve space for.
         */
        void reserve(std::size_t n) noexcept;

        /**
         * Add a new entity to the layout.
         *
         * @param i
         * The entity's index in the atlas.
         *
         * @param x
         * The starting horizontal position.
         *
         * @param y
         * The starting vertical position.
         *
         * @param z
         * The starting aerial position.
         */
        void spawn(core::index_t i, float x, float y, float z) noexcept;
        void spawn(assets::sprite32_id i, float x, float y, float z) noexcept;

        // Accessors

        [[nodiscard]] constexpr auto xform_buffer() const noexcept
                -> const MTL::Buffer* __restrict;

        [[nodiscard]] constexpr auto index_buffer() const noexcept
                -> const MTL::Buffer* __restrict;

        SC_REGISTER_SWAPPABLE_ACCESSOR(x_pos, new_x_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(y_pos, new_y_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(z_pos, new_z_pos)

        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(x_vel)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(y_vel)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(z_vel)

        SC_REGISTER_SWAPPABLE_ACCESSOR(new_x_pos, x_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(new_y_pos, y_pos)
        SC_REGISTER_SWAPPABLE_ACCESSOR(new_z_pos, z_pos)

        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(sprite_index)
        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(draw_order)
        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(next_in_cell)

        [[nodiscard]] constexpr std::size_t count() const noexcept;
        [[nodiscard]] constexpr std::size_t capacity() const noexcept;

        [[nodiscard]] constexpr std::size_t offset(
                xform_channel) const noexcept;

        [[nodiscard]] constexpr std::size_t offset(
                index_channel) const noexcept;

        // Attributes

        bool draw_order_needs_sort{false};

    private:
        bool channels_swapped_{false};

        // Type aliases

        template<typename T, bool IsConst>
        using ptr_t = std::conditional_t<IsConst, const T*, T*>;

        // Static methods

        template<typename EnumType, typename T, bool IsConst>
        [[nodiscard]] static constexpr auto get_ptr(
                const mem::channel_pool<T,
                        static_cast<std::size_t>(EnumType::count)>& buffer,
                EnumType channel) noexcept -> ptr_t<T, IsConst> __restrict;

        // Attributes

        mem::channel_pool<float, static_cast<std::size_t>(xform_channel::count)>
                xform_buffer_;

        mem::channel_pool<core::index_t,
                static_cast<std::size_t>(index_channel::count)>
                index_buffer_;

        MTL::Device* device_{nullptr};
    };

    // Constructors

    [[nodiscard]] constexpr entity_registry::entity_registry(
            MTL::Device* device)
        : device_{device}
    {
        reserve(core::kCacheAlignment);
    }

    // Mutators

    inline void entity_registry::commit() noexcept
    {
        channels_swapped_ = !channels_swapped_;
    }

    // Accessors

    [[nodiscard]] constexpr std::size_t entity_registry::count() const noexcept
    {
        return xform_buffer_.count;
    }

    [[nodiscard]] constexpr std::size_t
    entity_registry::capacity() const noexcept
    {
        return xform_buffer_.capacity;
    }

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            xform_channel channel) const noexcept
    {
        return xform_buffer_.subarray_offset(static_cast<std::size_t>(channel));
    }

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            index_channel channel) const noexcept
    {
        return index_buffer_.subarray_offset(static_cast<std::size_t>(channel));
    }

    [[nodiscard]] constexpr auto entity_registry::xform_buffer() const noexcept
            -> const MTL::Buffer* __restrict
    {
        return xform_buffer_.buffer.get();
    }

    [[nodiscard]] constexpr auto entity_registry::index_buffer() const noexcept
            -> const MTL::Buffer* __restrict
    {
        return index_buffer_.buffer.get();
    }

    // Private helpers

    template<typename EnumType, typename T, bool IsConst>
    [[nodiscard]] constexpr auto entity_registry::get_ptr(
            const mem::channel_pool<T,
                    static_cast<std::size_t>(EnumType::count)>& buffer,
            EnumType channel) noexcept -> ptr_t<T, IsConst> __restrict
    {
        return buffer[static_cast<std::size_t>(channel)];
    }

} // namespace sc

std::ostream& operator<<(
        std::ostream& out, const sc::entity_registry& registry);

#undef SC_REGISTER_SWAPPABLE_ACCESSOR
#undef SC_REGISTER_INDEX_CHANNEL_ACCESSOR
#undef SC_REGISTER_XFORM_CHANNEL_ACCESSOR
#undef SC_REGISTER_ACCESSOR

#endif // SC_REGISTRY_SCENE_REGISTRY_HH
