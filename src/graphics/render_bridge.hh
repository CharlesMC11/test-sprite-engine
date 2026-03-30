#pragma once

#include <Metal/Metal.hpp>

#include "atlas.hh"
#include "entity_registry.hh"
#include "mapped_view.hh"

namespace sc {

    /**
     * High-level interface for the Metal Compute pipeline.
     *
     * Orchestrates the dispatch of kernels to the GPU using `sc::sprite` data
     * as primary input.
     */
    class render_bridge final {
    public:
        // Constructors

        explicit render_bridge(MTL::Device* device);

        render_bridge(const render_bridge&) = delete;
        render_bridge& operator=(const render_bridge&) = delete;

        render_bridge(render_bridge&&) = delete;
        render_bridge& operator=(render_bridge&&) = delete;

        ~render_bridge() = default;

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
         *
         */
        void clear() const;

        /**
         * Encode a draw command for entities on screen.
         *
         * @param registry
         *
         */
        void draw(const entity_registry& registry) const;

        // Mutators

        /**
         *
         * @param view
         *
         */
        void set_sprite_atlas(const core::mapped_view<sprites::atlas>& view);

    private:
        // Attributes

        NS::SharedPtr<MTL::Device> device_;
        NS::SharedPtr<MTL::CommandQueue> queue_;

        NS::SharedPtr<MTL::ComputePipelineState> clear_pso_{nullptr};
        NS::SharedPtr<MTL::ComputePipelineState> sprite_pso_{nullptr};

        NS::SharedPtr<MTL::Buffer> sprite32_buffer_{nullptr};
        std::size_t palette_span_offset_{0u};
        std::size_t sprite32_span_offset_{0u};

        MTL::CommandBuffer* command_buffer_{nullptr};
        MTL::ComputeCommandEncoder* encoder_{nullptr};
    };

} // namespace sc
