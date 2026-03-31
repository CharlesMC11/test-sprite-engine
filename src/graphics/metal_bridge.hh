#ifndef SC_GRAPHICS_RENDER_BRIDGE_HH
#define SC_GRAPHICS_RENDER_BRIDGE_HH

#include <Metal/Metal.hpp>

#include "assets/atlas.hh"
#include "core/mapped_view.hh"
#include "registry/entity_registry.hh"

namespace sc::render {

    /**
     * High-level interface for the Metal Compute pipeline.
     *
     * Orchestrates the dispatch of kernels to the GPU using
     * `sc::assets::sprite` data as primary input.
     */
    class metal_bridge final {
    public:
        // Constructors

        explicit metal_bridge(MTL::Device* device);

        metal_bridge(const metal_bridge&) = delete;
        metal_bridge& operator=(const metal_bridge&) = delete;

        metal_bridge(metal_bridge&&) = delete;
        metal_bridge& operator=(metal_bridge&&) = delete;

        ~metal_bridge() = default;

        // Public methods

        /**
         *
         *
         * @param buffer
         * The destination drawable/texture.
         */
        void begin_frame(const MTL::Drawable* buffer);

        /**
         *
         *
         * @param buffer
         * The destination drawable/texture.
         */
        void end_frame(const MTL::Drawable* buffer);

        /**
         * Encode a command to clear the screen.
         */
        void clear() const;

        /**
         * Encode a draw command for entities on screen.
         *
         * @param registry
         * The registry containing the entities to draw.
         */
        void draw(const entity_registry& registry) const;

        // Mutators

        /**
         * Create a shared buffer for the atlas.
         *
         * @param atlas
         * The mapped view of atlas to store in the buffer.
         */
        void set_atlas_buffer(const core::mapped_view<assets::atlas>& atlas);

    private:
        // Attributes

        NS::SharedPtr<MTL::Device> device_;
        NS::SharedPtr<MTL::CommandQueue> queue_;

        NS::SharedPtr<MTL::ComputePipelineState> clear_pso_{nullptr};
        NS::SharedPtr<MTL::ComputePipelineState> sprite_pso_{nullptr};

        NS::SharedPtr<MTL::Buffer> sprite32_buffer_{nullptr};
        std::size_t palette_span_offset_{0UZ};
        std::size_t sprite32_span_offset_{0UZ};

        MTL::CommandBuffer* command_buffer_{nullptr};
        MTL::ComputeCommandEncoder* encoder_{nullptr};
    };

} // namespace sc::render

#endif // SC_GRAPHICS_RENDER_BRIDGE_HH
