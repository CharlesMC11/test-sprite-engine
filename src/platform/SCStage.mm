#import "SCStage.hh"

#import <MetalKit/MetalKit.h>

#include <cstdint>
#include <memory>

#include "atlas.hh"
#include "core.hh"
#include "input.hh"
#include "mapped_view.hh"
#include "physics.hh"
#include "render_bridge.hh"
#include "scene_registry.hh"
#include "sprite.hh"
#include "sprite_index.hh"

@implementation SCStage {
    std::unique_ptr<sc::core::mapped_view<sc::sprites::atlas>> _view;
    std::unique_ptr<sc::render_bridge> _bridge;
    const sc::sprites::atlas* _atlas;
    sc::scene_registry _registry;
    sc::core::input_mask _keysPressed;
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

        _view = std::make_unique<sc::core::mapped_view<sc::sprites::atlas>>(
                sc::assets::kCharacterAtlas);
        if (!(_view && *_view)) {
            NSLog(@"FATAL: Could not map sprite bank file.");
            abort();
        }
        if (!sc::sprites::atlas::validate(_view->data(), _view->size())) {
            NSLog(@"FATAL: sprite bank header validation failed.");
            abort();
        }
        _atlas = _view->data();

        _bridge = std::make_unique<sc::render_bridge>(
                (__bridge MTL::Device*) device);
        _bridge->set_sprite_atlas(*_view);

        self.framebufferOnly = false;

        constexpr auto id{sc::sprites::sprite_index::LANCIS};
        const sc::sprites::metadata& sprite{(*_atlas)[id].meta};
        _registry.spawn(
                (sc::display::kWidth - sc::sprites::kWidth - sprite.anchor_x) *
                        0.5f,
                (sc::display::kHeight - sc::sprites::kHeight -
                        sprite.anchor_y) *
                        0.5f,
                0.0f, id);

        _registry.spawn(sc::display::kWidth * 0.25f,
                sc::display::kHeight * 0.25, 0.0f,
                sc::sprites::sprite_index::MYARRA);

        _registry.spawn(sc::display::kWidth * 0.75f,
                sc::display::kHeight * 0.75f, 0.0f,
                sc::sprites::sprite_index::HEART_OW_F);
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
        _registry.vec_x_ptr()[0] = _registry.vec_y_ptr()[0] = 0;
        if (_keysPressed & sc::input::mask::UP)
            _registry.vec_y_ptr()[0] -= speed;
        if (_keysPressed & sc::input::mask::DOWN)
            _registry.vec_y_ptr()[0] += speed;
        if (_keysPressed & sc::input::mask::LEFT)
            _registry.vec_x_ptr()[0] -= speed;
        if (_keysPressed & sc::input::mask::RIGHT)
            _registry.vec_x_ptr()[0] += speed;

        _registry.update(sc::physics::kFixedTimestep);
        sc::physics::resolve_entity_collisions(
                *_atlas, _registry, sc::physics::kFixedTimestep);
        _registry.commit();

        _accumulator -= sc::physics::kFixedTimestep;
    }

    _registry.print();

    _registry.sort_draw();
    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _bridge->begin_frame(drawable);
    _bridge->clear();
    _bridge->draw(_registry);
    _bridge->end_frame(drawable);
}

@end
