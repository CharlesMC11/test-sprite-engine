#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "render_bridge.hh"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

#include "atlas.hh"
#include "core.hh"
#include "mapped_view.hh"

namespace sc {

    render_bridge::render_bridge(MTL::Device* device)
        : device_{NS::TransferPtr(device)},
          queue_{NS::TransferPtr(device_->newCommandQueue())}
    {
        NS::Error* error{nullptr};

        const auto* library_path{
                NS::String::string(assets::kShaderLib, NS::UTF8StringEncoding)};
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

    void render_bridge::set_sprite_atlas(
            const core::mapped_view<sprites::atlas>& view)
    {
        constexpr std::size_t metadata_size{sizeof(view->meta)};
        const std::size_t palette_size{
                sizeof(sprites::palette) * view->meta.palette_count};
        const std::size_t sprite_size{
                sizeof(sprites::sprite32x32) * view->meta.sprite_count};
        const std::size_t total_size{sizeof(view) + palette_size + sprite_size};

        sprite_buffer_ = NS::TransferPtr(device_->newBuffer(view.data(),
                total_size, MTL::ResourceStorageModeShared, nullptr));

        palette_offset_ = metadata_size;
        sprites_offset_ = metadata_size + palette_size;
    }

    void render_bridge::begin_frame(const MTL::Drawable* buffer)
    {
        command_buffer_ = queue_->commandBuffer();
        encoder_ = command_buffer_->computeCommandEncoder();

        const auto* out_texture{
                reinterpret_cast<const CA::MetalDrawable*>(buffer)->texture()};
        encoder_->setTexture(out_texture, 0);
    }

    void render_bridge::end_frame(const MTL::Drawable* buffer)
    {
        encoder_->endEncoding();
        command_buffer_->presentDrawable(buffer);
        command_buffer_->commit();

        encoder_ = nullptr;
        command_buffer_ = nullptr;
    }

    void render_bridge::clear() const
    {
        encoder_->setComputePipelineState(clear_pso_.get());

        const auto grid_size = MTL::Size(display::kWidth, display::kHeight, 1);
        const auto thread_group_size = MTL::Size(16, 16, 1);

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void render_bridge::draw(const scene_registry& registry) const
    {
        encoder_->setComputePipelineState(sprite_pso_.get());

        encoder_->setBuffer(sprite_buffer_.get(), palette_offset_, 0u);
        encoder_->setBuffer(sprite_buffer_.get(), sprites_offset_, 1u);

        encoder_->setBytes(
                registry.pos_x_ptr(), sizeof(float) * registry.count(), 2u);
        encoder_->setBytes(
                registry.pos_y_ptr(), sizeof(float) * registry.count(), 3u);
        encoder_->setBytes(
                registry.pos_z_ptr(), sizeof(float) * registry.count(), 4u);

        encoder_->setBytes(registry.indices.data(),
                sizeof(core::atlas_index) * registry.count(), 5u);
        encoder_->setBytes(registry.draw_order.data(),
                sizeof(core::index_t) * registry.count(), 6u);

        const auto count{static_cast<std::uint32_t>(registry.count())};
        encoder_->setBytes(&count, sizeof(count), 7u);

        const MTL::Size grid_size{display::kWidth, display::kHeight, 1u};
        const MTL::Size thread_group_size{16u, 16u, 1u};

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

} // namespace sc
