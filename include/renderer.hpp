/**
 * @file renderer.hpp
 * @brief
 */
#pragma once

#include <Metal/Metal.hpp>

#include "atlas.hpp"
#include "sprite.hpp"
#include "transform_registry.hpp"

namespace sc {

    /**
     * @class renderer
     * @brief High-level interface for the Metal Compute pipeline.
     *
     * Orchestrates the dispatch of kernels to the GPU using `sc::sprite` data
     * as primary input.
     */
    class renderer final {
    public:
        explicit renderer(MTL::Device* device);
        ~renderer() = default;

        renderer(const renderer&) = delete;
        renderer(renderer&&) = delete;
        renderer& operator=(const renderer&) = delete;
        renderer& operator=(renderer&&) = delete;

        /**
         * @brief
         * @param drawable The destination drawable/texture.
         */
        void begin_frame(const MTL::Drawable* drawable);

        void clear(const MTL::Drawable* drawable);

        /**
         * @brief Encode a draw command for a single sprite.
         * @param sprite The source data to render.
         * @param pos_x
         * @param pos_y
         */
        void draw(const sprite& sprite, float pos_x, float pos_y) const;

        void draw(const transform_registry& registry, const atlas& atlas) const;

        /**
         * @brief
         */
        void end_frame(const MTL::Drawable* drawable);

    private:
        NS::SharedPtr<MTL::Device> device_;
        NS::SharedPtr<MTL::CommandQueue> command_queue_;
        NS::SharedPtr<MTL::ComputePipelineState> clear_pso_{nullptr};
        NS::SharedPtr<MTL::ComputePipelineState> draw_pso_{nullptr};

        MTL::CommandBuffer* buffer_{nullptr};
        MTL::ComputeCommandEncoder* encoder_{nullptr};
    };

} // namespace sc
