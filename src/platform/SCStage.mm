#import "SCStage.hh"

#import <MetalKit/MetalKit.h>

#include <cstdint>
#include <memory>

#include "assets/asset_constants.hh"
#include "assets/atlas.hh"
#include "assets/sprite.hh"
#include "assets/sprite32_index.hh"
#include "core/core.hh"
#include "core/input.hh"
#include "core/mapped_view.hh"
#include "graphics/display_constants.hh"
#include "graphics/render_bridge.hh"
#include "registry/entity_registry.hh"
#include "simulation/physics.hh"

@implementation SCStage {
    std::unique_ptr<sc::core::mapped_view<sc::assets::atlas>> _view;
    std::unique_ptr<sc::entity_registry> _registry;
    std::unique_ptr<sc::render_bridge> _bridge;
    const sc::assets::atlas* _atlas;
    sc::input::mask _keysPressed;
    float _accumulator;
}

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frameRect device:device];
    if (self) {
        self.delegate = self;
        self.drawableSize =
                CGSizeMake(sc::display::kWidth, sc::display::kHeight);
        self.preferredFramesPerSecond = sc::display::kTargetFPS;

        self.layer.magnificationFilter = kCAFilterNearest;

        _view = std::make_unique<sc::core::mapped_view<sc::assets::atlas>>(
                sc::assets::kCharacterAtlas);
        if (!(_view && *_view)) {
            NSLog(@"FATAL: Could not map sprite bank file.");
            abort();
        }
        if (!sc::assets::atlas::validate(_view->data(), _view->size())) {
            NSLog(@"FATAL: sprite bank header validation failed.");
            abort();
        }
        _atlas = _view->data();

        _registry = std::make_unique<sc::entity_registry>(
                (__bridge MTL::Device*) device);

        _bridge = std::make_unique<sc::render_bridge>(
                (__bridge MTL::Device*) device);
        _bridge->set_sprite_atlas(*_view);

        self.framebufferOnly = false;

        constexpr auto id{sc::assets::sprite32_index::LANCIS};
        const sc::assets::sprites::metadata& sprite{(*_atlas)[id].meta};
        _registry->spawn((sc::display::kWidth - 32u - sprite.origin_u) * 0.5f,
                (sc::display::kHeight - 32u - sprite.origin_v) * 0.5f, 0.0f,
                id);

        _registry->spawn(0.0f, 0.0f, 0.0f, sc::assets::sprite32_index::MYARRA);

        _registry->spawn(sc::display::kWidth * 0.75f,
                sc::display::kHeight * 0.75f, 26.0f,
                sc::assets::sprite32_index::HEART_OW_F);
    }

    return self;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(nonnull NSEvent*)event
{
    [self.window makeFirstResponder:self];
}

- (void)keyDown:(nonnull NSEvent*)event
{
    switch (event.keyCode) {
    case 13:
        _keysPressed |= sc::input::mask::UP;
        break;
    case 1:
        _keysPressed |= sc::input::mask::DOWN;
        break;
    case 0:
        _keysPressed |= sc::input::mask::LEFT;
        break;
    case 2:
        _keysPressed |= sc::input::mask::RIGHT;
        break;
    default:
        break;
    }
}

- (void)keyUp:(nonnull NSEvent*)event
{
    switch (event.keyCode) {
    case 13:
        _keysPressed &= ~sc::input::mask::UP;
        break;
    case 1:
        _keysPressed &= ~sc::input::mask::DOWN;
        break;
    case 0:
        _keysPressed &= ~sc::input::mask::LEFT;
        break;
    case 2:
        _keysPressed &= ~sc::input::mask::RIGHT;
        break;
    default:
        break;
    }
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

- (void)drawInMTKView:(nonnull MTKView*)view
{
    float frameTime{1.0f / (float) view.preferredFramesPerSecond};
    if (frameTime > 0.25f)
        frameTime = 0.25f;

    _accumulator += frameTime;

    float speed{200.0f};
    while (_accumulator >= sc::physics::kFixedTimestep) {
        _registry->vel_x_ptr()[0] = _registry->vel_y_ptr()[0] = 0;
        if (sc::core::any(_keysPressed & sc::input::mask::UP))
            _registry->vel_y_ptr()[0] -= speed;
        if (sc::core::any(_keysPressed & sc::input::mask::DOWN))
            _registry->vel_y_ptr()[0] += speed;
        if (sc::core::any(_keysPressed & sc::input::mask::LEFT))
            _registry->vel_x_ptr()[0] -= speed;
        if (sc::core::any(_keysPressed & sc::input::mask::RIGHT))
            _registry->vel_x_ptr()[0] += speed;

        _registry->update(sc::physics::kFixedTimestep);
        sc::physics::resolve_entity_collisions(
                *_atlas, *_registry, sc::physics::kFixedTimestep);
        _registry->commit();

        _accumulator -= sc::physics::kFixedTimestep;
    }

    _registry->sort_draw();
    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _bridge->begin_frame(drawable);
    _bridge->clear();
    _bridge->draw(*_registry);
    _bridge->end_frame(drawable);
}

@end
