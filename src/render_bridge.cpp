#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "render_bridge.hpp"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

#include "constants.hpp"
#include "entity_id.hpp"

namespace sc {

    render_bridge::render_bridge(MTL::Device* device)
        : device_{NS::TransferPtr(device)},
          queue_{NS::TransferPtr(device_->newCommandQueue())}
    {
        NS::Error* error{nullptr};

        const auto* library_path{
                NS::String::string(assets::SHADER_LIB, NS::UTF8StringEncoding)};
        auto* library{device->newLibrary(library_path, &error)};

        const auto* fn_name{
                NS::String::string("k_clear_screen", NS::UTF8StringEncoding)};
        auto* function{library->newFunction(fn_name)};

        clear_pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));
        if (!clear_pso_) [[unlikely]]
            std::cerr << error->localizedDescription() << std::endl;

        fn_name = NS::String::string("k_draw_sprites", NS::UTF8StringEncoding);
        function = library->newFunction(fn_name);

        sprite_pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));
        if (!sprite_pso_) [[unlikely]]
            std::cerr << error->localizedDescription() << std::endl;

        function->release();
        library->release();
    }

    void render_bridge::begin_frame(const MTL::Drawable* buffer)
    {
        buffer_ = queue_->commandBuffer();
        encoder_ = buffer_->computeCommandEncoder();
        encoder_->setComputePipelineState(sprite_pso_.get());

        const auto* out_texture{
                reinterpret_cast<const CA::MetalDrawable*>(buffer)->texture()};
        encoder_->setTexture(out_texture, 0);
    }

    // FIXME
    void render_bridge::clear(const MTL::Drawable* buffer)
    {
        auto* out_texture =
                reinterpret_cast<const
                CA::MetalDrawable*>(buffer)->texture();

        // 1. Switch to the Clear PSO
        encoder_->setComputePipelineState(clear_pso_.get());
        encoder_->setTexture(out_texture, 0);

        // 2. Dispatch across the whole 240x160 screen
        const auto grid_size =
                MTL::Size(display::SCREEN_WIDTH, display::SCREEN_HEIGHT, 1);
        const auto thread_group_size =
                MTL::Size(16, 16, 1); // Standard block size

        encoder_->dispatchThreads(grid_size, thread_group_size);

        // 3. Switch back to the Registry PSO for the next calls
        encoder_->setComputePipelineState(sprite_pso_.get());
    }

    void render_bridge::draw(
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

    void render_bridge::draw(
            const sprite_bank& sprites, const entity_layout& layout) const
    {
        encoder_->setBytes(&sprites[0], sizeof(sprite) * sprites.size(), 0);
        encoder_->setBytes(layout.x.data(), sizeof(float) * layout.size(), 1);
        encoder_->setBytes(layout.y.data(), sizeof(float) * layout.size(), 2);
        encoder_->setBytes(
                layout.entity_ids.data(), sizeof(entity_id) * layout.size(), 3);

        const MTL::Size grid_size{
                SPRITE_WIDTH * layout.size(), SPRITE_HEIGHT, 1};
        const MTL::Size thread_group_size{SPRITE_WIDTH, SPRITE_HEIGHT, 1};

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void render_bridge::end_frame(const MTL::Drawable* buffer)
    {
        encoder_->endEncoding();
        buffer_->presentDrawable(buffer);
        buffer_->commit();

        encoder_ = nullptr;
        buffer_ = nullptr;
    }

} // namespace sc
