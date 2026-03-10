#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "render_bridge.hh"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

#include "core.hh"

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

    void render_bridge::set_sprite_atlas(const sprites::atlas& atlas)
    {
        const std::size_t size{sizeof(sprites::sprite) * atlas.size()};

        sprite_buffer_ = NS::TransferPtr(
                device_->newBuffer(size, MTL::ResourceStorageModeShared));

        std::memcpy(sprite_buffer_->contents(), &atlas[0], size);
    }

    void render_bridge::begin_frame(const MTL::Drawable* buffer)
    {
        command_buffer_ = queue_->commandBuffer();
        encoder_ = command_buffer_->computeCommandEncoder();

        const auto* out_texture{
                reinterpret_cast<const CA::MetalDrawable*>(buffer)->texture()};
        encoder_->setTexture(out_texture, 0);
    }

    void render_bridge::clear() const
    {
        encoder_->setComputePipelineState(clear_pso_.get());

        const auto grid_size = MTL::Size(display::kWidth, display::kHeight, 1);
        const auto thread_group_size = MTL::Size(16, 16, 1);

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void render_bridge::draw(const scene_population& registry) const
    {
        encoder_->setComputePipelineState(sprite_pso_.get());

        encoder_->setBuffer(sprite_buffer_.get(), 0, 0);
        encoder_->setBytes(
                registry.x.data(), sizeof(float) * registry.size(), 1);
        encoder_->setBytes(
                registry.y.data(), sizeof(float) * registry.size(), 2);
        encoder_->setBytes(
                registry.z.data(), sizeof(float) * registry.size(), 3);
        encoder_->setBytes(registry.indices.data(),
                sizeof(sys::atlas_index_t) * registry.size(), 4);
        encoder_->setBytes(registry.draw_order.data(),
                sizeof(sys::index_t) * registry.size(), 5);

        const auto count{static_cast<std::uint32_t>(registry.size())};
        encoder_->setBytes(&count, sizeof(count), 6);

        const MTL::Size grid_size{display::kWidth, display::kHeight, 1};
        const MTL::Size thread_group_size{16, 16, 1};

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void render_bridge::end_frame(const MTL::Drawable* buffer)
    {
        encoder_->endEncoding();
        command_buffer_->presentDrawable(buffer);
        command_buffer_->commit();

        encoder_ = nullptr;
        command_buffer_ = nullptr;
    }

} // namespace sc
