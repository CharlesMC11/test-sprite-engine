#include "world.hh"

#include <stdexcept>

#include "assets/asset_constants.hh"
#include "graphics/display_constants.hh"
#include "simulation/physics.hh"
#include "simulation/physics_types.hh"

namespace sc {

    [[nodiscard]] world::world(MTL::Device* device)
        : grid_{}, bridge_{render::metal_bridge{device}},
          registry_{entity_registry{device}},
          atlas_{core::mapped_view<assets::atlas>{assets::kAtlas}}
    {
        if (!(atlas_ && atlas_.data()))
            throw std::runtime_error{"Could not load the atlas."};

        if (!assets::atlas::validate(atlas_.data(), atlas_.size()))
            throw std::runtime_error{"Atlas has an invalid structure."};

        bridge_.set_atlas_buffer(atlas_);

        constexpr auto idx{assets::sprite32_index::LANCIS};
        const assets::sprites::metadata sprite{atlas_->operator[](idx).meta};
        registry_.spawn(idx, (display::kWidth - 32U - sprite.u_anchor) * 0.5f,
                (display::kHeight - 32U - sprite.v_anchor) * 0.5f, 0.0f);

        registry_.spawn(assets::sprite32_index::MYARRA, 0.0f, 0.0f, 0.0f);

        registry_.spawn(assets::sprite32_index::HEART_OW_F,
                display::kWidth * 0.75f, display::kHeight * 0.75f, 25.0f);
    }

    void world::update(const input::mask input, const MTL::Drawable* drawable)
    {
        float frame_time{1.0f / display::kTargetFPS};
        if (frame_time > 0.25f)
            frame_time = 0.25f;

        ftime_accumulator_ += frame_time;

        while (ftime_accumulator_ >= physics::kFixedTimestep) {
            registry_.x_vel_ptr()[0UZ] = registry_.y_vel_ptr()[0UZ] = 0.0f;

            constexpr float vel{200.0f};
            if (core::any(input & input::mask::up))
                registry_.y_vel_ptr()[0UZ] -= vel;
            if (core::any(input & input::mask::down))
                registry_.y_vel_ptr()[0UZ] += vel;
            if (core::any(input & input::mask::left))
                registry_.x_vel_ptr()[0UZ] -= vel;
            if (core::any(input & input::mask::right))
                registry_.x_vel_ptr()[0UZ] += vel;

            registry_.update(physics::kFixedTimestep);
            grid_.update(registry_);
            physics::resolve_entity_collisions(
                    registry_, grid_, *atlas_.data(), physics::kFixedTimestep);

            registry_.commit();
            ftime_accumulator_ -= physics::kFixedTimestep;
        }

        registry_.sort_draw();

        bridge_.begin_frame(drawable);
        bridge_.clear();
        bridge_.draw(registry_);
        bridge_.end_frame(drawable);
    }

} // namespace sc
