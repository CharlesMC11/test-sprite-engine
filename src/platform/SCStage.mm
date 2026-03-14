#import "SCStage.hh"

#import <MetalKit/MetalKit.h>

#include <cstdint>
#include <memory>

#include "atlas.hh"
#include "atlas_index.hh"
#include "core.hh"
#include "file_mapping.hh"
#include "physics_engine.h"
#include "render_bridge.hh"
#include "scene_population.hh"
#include "sprite.hh"

@implementation SCStage {
    std::unique_ptr<sc::core::file_mapping<sc::sprites::atlas>> _mapper;
    std::unique_ptr<sc::render_bridge> _bridge;
    const sc::sprites::atlas* _atlas;
    sc::scene_population _registry;
    sc::core::input_mask_t _keysPressed;
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

        _mapper = std::make_unique<sc::core::file_mapping<sc::sprites::atlas>>(
                sc::assets::kCharacterAtlas);
        if (!(_mapper && *_mapper)) {
            NSLog(@"FATAL: Could not map sprite bank file.");
            abort();
        }
        if (!sc::sprites::atlas::validate(_mapper->data(), _mapper->size())) {
            NSLog(@"FATAL: sprite bank header validation failed.");
            abort();
        }
        _atlas = _mapper->data();

        _bridge = std::make_unique<sc::render_bridge>(
                (__bridge MTL::Device*) device);
        _bridge->set_sprite_atlas(*_atlas);

        self.framebufferOnly = false;

        constexpr auto id{sc::sprites::atlas_index::LANCIS};
        const sc::sprites::sprite& sprite{(*_atlas)[id]};
        _registry.spawn(
                (sc::display::kWidth - sc::sprites::kWidth - sprite.anchor_x) *
                        0.5f,
                (sc::display::kHeight - sc::sprites::kHeight -
                        sprite.anchor_y) *
                        0.5f,
                0.0f, id);

        _registry.spawn(sc::display::kHeight * 0.75f,
                sc::display::kHeight * 0.75f, 0.0f,
                sc::sprites::atlas_index::HEART);
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
        _keysPressed |= sc::input::kUp;
        break;
    case 1:
        _keysPressed |= sc::input::kDown;
        break;
    case 0:
        _keysPressed |= sc::input::kLeft;
        break;
    case 2:
        _keysPressed |= sc::input::kRight;
        break;
    }
}

- (void)keyUp:(nonnull NSEvent*)event
{
    switch (event.keyCode) {
    case 13:
        _keysPressed &= ~sc::input::kUp;
        break;
    case 1:
        _keysPressed &= ~sc::input::kDown;
        break;
    case 0:
        _keysPressed &= ~sc::input::kLeft;
        break;
    case 2:
        _keysPressed &= ~sc::input::kRight;
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
        _registry.dx[0] = _registry.dy[0] = 0;
        if (_keysPressed & sc::input::kUp)
            _registry.dy[0] -= speed;
        if (_keysPressed & sc::input::kDown)
            _registry.dy[0] += speed;
        if (_keysPressed & sc::input::kLeft)
            _registry.dx[0] -= speed;
        if (_keysPressed & sc::input::kRight)
            _registry.dx[0] += speed;

        _registry.update(sc::physics::kFixedTimestep);
        sc::resolve_entity_collisions(*_atlas, _registry);

        _accumulator -= sc::physics::kFixedTimestep;
    }

    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _bridge->begin_frame(drawable);
    _bridge->clear();
    _bridge->draw(_registry);
    _bridge->end_frame(drawable);
}

@end
