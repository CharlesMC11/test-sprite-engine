#ifndef SC_REGISTRY_SCENE_REGISTRY_HH
#define SC_REGISTRY_SCENE_REGISTRY_HH

#include <algorithm>
#include <cstddef>

#include "assets/sprite32_index.hh"
#include "core/core.hh"
#include "core/memory.hh"

/**
 * Register a channel accessor method for the registry.
 *
 * @param name
 * The name to use for the accessor.
 *
 * @param EnumType
 * The enum referred to by the buffer.
 *
 * @param DataType
 * The type contained in the buffer.
 *
 * @param ENUM_VAL
 * The enum value to access.
 *
 * @param buffer
 * The name of the buffer.
 */
#define SC_REGISTER_ACCESSOR(name, EnumType, DataType, ENUM_VAL, buffer)       \
    [[nodiscard]] constexpr auto name##_ptr() noexcept -> DataType* __restrict \
    {                                                                          \
        return get_ptr<EnumType, DataType, false>(buffer, EnumType::ENUM_VAL); \
    }                                                                          \
                                                                               \
    [[nodiscard]] constexpr auto name##_ptr() const noexcept                   \
            -> const DataType* __restrict                                      \
    {                                                                          \
        return get_ptr<EnumType, DataType, true>(buffer, EnumType::ENUM_VAL);  \
    }

#define SC_REGISTER_XFORM_CHANNEL_ACCESSOR(name, ENUM_VAL)                     \
    SC_REGISTER_ACCESSOR(name, xform_channel, float, ENUM_VAL, float_buffer_)

#define SC_REGISTER_INDEX_CHANNEL_ACCESSOR(name, ENUM_VAL)                     \
    SC_REGISTER_ACCESSOR(                                                      \
            name, index_channel, core::index_t, ENUM_VAL, index_buffer_)

namespace sc {

    /**
     * @struct entity_registry
     * @brief
     */
    class entity_registry final {
    public:
        // Enums

        enum class xform_channel : std::uint8_t {
            X_POSITION,
            Y_POSITION,
            Z_POSITION,
            X_VELOCITY,
            Y_VELOCITY,
            Z_VELOCITY,
            X_TARGET_POSITION,
            Y_TARGET_POSITION,
            Z_TARGET_POSITION,
            COUNT
        };

        enum class index_channel : std::uint8_t {
            SPRITE32_INDEX,
            PHYSICS_ORDER,
            DRAW_ORDER,
            NEXT_IN_CELL,
            COUNT
        };

        // Constructors

        [[nodiscard]] explicit constexpr entity_registry(MTL::Device* device);

        entity_registry(const entity_registry&) = delete;
        entity_registry& operator=(const entity_registry&) = delete;

        entity_registry(entity_registry&&) = default;
        entity_registry& operator=(entity_registry&&) noexcept = default;

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

        /**
         * Print the entries in the registry.
         */
        void print() const;

        // Mutators

        /**
         * Reserve space in the registry.
         *
         * @param n
         * The number of entries to reserve.
         */
        void reserve(std::size_t n);

        /**
         * Add a new entity to the layout.
         *
         * @param start_x
         * The starting horizontal position.
         *
         * @param start_y
         * The starting vertical position.
         *
         * @param start_z
         * The starting aerial position.
         *
         * @param i
         * The entity's index in the atlas.
         */
        void spawn(float start_x, float start_y, float start_z,
                assets::sprite32_index i);

        // Accessors

        [[nodiscard]] constexpr auto xform_buffer() const noexcept
                -> const MTL::Buffer* __restrict;

        [[nodiscard]] constexpr auto index_buffer() const noexcept
                -> const MTL::Buffer* __restrict;

        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(pos_x, X_POSITION)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(pos_y, Y_POSITION)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(pos_z, Z_POSITION)

        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(vel_x, X_VELOCITY)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(vel_y, Y_VELOCITY)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(vel_z, Z_VELOCITY)

        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(new_x, X_TARGET_POSITION)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(new_y, Y_TARGET_POSITION)
        SC_REGISTER_XFORM_CHANNEL_ACCESSOR(new_z, Z_TARGET_POSITION)

        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(sprite32_index, SPRITE32_INDEX)
        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(physics_order, PHYSICS_ORDER)
        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(draw_order, DRAW_ORDER)
        SC_REGISTER_INDEX_CHANNEL_ACCESSOR(next_in_cell, NEXT_IN_CELL)

        [[nodiscard]] constexpr std::size_t count() const noexcept;
        [[nodiscard]] constexpr std::size_t capacity() const noexcept;

        [[nodiscard]] constexpr std::size_t offset(
                xform_channel) const noexcept;

        [[nodiscard]] constexpr std::size_t offset(
                index_channel) const noexcept;

        // Attributes

        bool needs_sort{false};

    private:
        // Type aliases

        template<typename T, bool IsConst>
        using ptr_t = std::conditional_t<IsConst, const T*, T*>;

        // Static methods

        template<typename E, typename T, bool IsConst>
        [[nodiscard]] static constexpr auto get_ptr(
                const mem::channel_pool<T, static_cast<std::size_t>(E::COUNT)>&
                        buffer,
                E channel) noexcept -> ptr_t<T, IsConst> __restrict;

        // Attributes

        mem::channel_pool<float, static_cast<std::size_t>(xform_channel::COUNT)>
                float_buffer_;
        mem::channel_pool<core::index_t,
                static_cast<std::size_t>(index_channel::COUNT)>
                index_buffer_;
        MTL::Device* device_{nullptr};
    };

    // Constructors

    constexpr entity_registry::entity_registry(MTL::Device* device)
        : device_{device}
    {
        reserve(core::kCacheAlignment);
    }

    // Accessors

    [[nodiscard]] constexpr std::size_t entity_registry::count() const noexcept
    {
        return float_buffer_.count;
    }

    [[nodiscard]] constexpr std::size_t
    entity_registry::capacity() const noexcept
    {
        return float_buffer_.capacity;
    }

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            xform_channel channel) const noexcept
    {
        return float_buffer_.subarray_offset(static_cast<std::size_t>(channel));
    }

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            index_channel channel) const noexcept
    {
        return index_buffer_.subarray_offset(static_cast<std::size_t>(channel));
    }

    [[nodiscard]] constexpr auto entity_registry::xform_buffer() const noexcept
            -> const MTL::Buffer* __restrict
    {
        return float_buffer_.buffer.get();
    }

    [[nodiscard]] constexpr auto entity_registry::index_buffer() const noexcept
            -> const MTL::Buffer* __restrict
    {
        return index_buffer_.buffer.get();
    }

    // Private helpers

    template<typename Channel, typename T, bool IsConst>
    [[nodiscard]] constexpr auto entity_registry::get_ptr(
            const mem::channel_pool<T,
                    static_cast<std::size_t>(Channel::COUNT)>& buffer,
            Channel channel) noexcept -> ptr_t<T, IsConst> __restrict
    {
        return buffer[static_cast<std::size_t>(channel)];
    }

} // namespace sc

#undef SC_REGISTER_INDEX_CHANNEL_ACCESSOR
#undef SC_REGISTER_XFORM_CHANNEL_ACCESSOR
#undef SC_REGISTER_ACCESSOR

#endif // SC_REGISTRY_SCENE_REGISTRY_HH
