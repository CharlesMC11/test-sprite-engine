#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "renderer.hpp"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

#include "atlas_index.hpp"
#include "constants.hpp"

namespace sc {

    renderer::renderer(MTL::Device* device)
        : device_{NS::TransferPtr(device)},
          command_queue_{NS::TransferPtr(device_->newCommandQueue())}
    {
        NS::Error* error{nullptr};

        const auto* library_path{
                NS::String::string(paths::SHADER_LIB, NS::UTF8StringEncoding)};
        auto* library{device->newLibrary(library_path, &error)};

        auto* fn_name{
                NS::String::string("render_registry", NS::UTF8StringEncoding)};
        auto* function{library->newFunction(fn_name)};

        draw_pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));
        if (!draw_pso_) [[unlikely]]
            std::cerr << error->localizedDescription() << std::endl;

        fn_name = NS::String::string("clear_screen", NS::UTF8StringEncoding);
        function = library->newFunction(fn_name);

        clear_pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));
        if (!clear_pso_) [[unlikely]]
            std::cerr << error->localizedDescription() << std::endl;

        function->release();
        library->release();
    }

    void renderer::begin_frame(const MTL::Drawable* drawable)
    {
        buffer_ = command_queue_->commandBuffer();
        encoder_ = buffer_->computeCommandEncoder();
        encoder_->setComputePipelineState(draw_pso_.get());

        const auto* out_texture{
                reinterpret_cast<const CA::MetalDrawable*>(drawable)
                        ->texture()};
        encoder_->setTexture(out_texture, 0);
    }

    // FIXME
    void renderer::clear(const MTL::Drawable* drawable)
    {
        auto* out_texture =
                reinterpret_cast<const CA::MetalDrawable*>(drawable)->texture();

        // 1. Switch to the Clear PSO
        encoder_->setComputePipelineState(clear_pso_.get());
        encoder_->setTexture(out_texture, 0);

        // 2. Dispatch across the whole 240x160 screen
        MTL::Size grid_size = MTL::Size(ui::SCREEN_WIDTH, ui::SCREEN_HEIGHT, 1);
        MTL::Size thread_group_size =
                MTL::Size(16, 16, 1); // Standard block size

        encoder_->dispatchThreads(grid_size, thread_group_size);

        // 3. Switch back to the Registry PSO for the next calls
        encoder_->setComputePipelineState(clear_pso_.get());
    }

    void renderer::draw(
            const sprite& sprite, const float pos_x, const float pos_y) const
    {
        encoder_->setBytes(&sprite, sizeof(sprite), 0);

        const struct {
            float x;
            float y;
        } pos{pos_x, pos_y};
        encoder_->setBytes(&pos, sizeof(pos), 1);

        const auto grid_size{MTL::Size(SPRITE_WIDTH, SPRITE_HEIGHT, 1)};
        const auto thread_group_size{MTL::Size(8, 8, 1)};
        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void renderer::draw(
            const transform_registry& registry, const atlas& atlas) const
    {
        encoder_->setBytes(&atlas[0], sizeof(sprite) * atlas.size(), 0);
        encoder_->setBytes(
                registry.x.data(), sizeof(float) * registry.size(), 1);
        encoder_->setBytes(
                registry.y.data(), sizeof(float) * registry.size(), 2);
        encoder_->setBytes(registry.sprite_ids.data(),
                sizeof(atlas_index) * registry.size(), 3);

        const MTL::Size grid_size{
                SPRITE_WIDTH * registry.size(), SPRITE_HEIGHT, 1};
        const MTL::Size thread_group_size{SPRITE_WIDTH, SPRITE_HEIGHT, 1};

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void renderer::end_frame(const MTL::Drawable* drawable)
    {
        encoder_->endEncoding();
        buffer_->presentDrawable(drawable);
        buffer_->commit();

        encoder_ = nullptr;
        buffer_ = nullptr;
    }

} // namespace sc
