#import "SCStage.hh"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "atlas.hh"
#include "atlas_index.hh"
#include "core.hh"
#include "file_mapping.hh"
#include "render_bridge.hh"
#include "scene_population.hh"
#include "sprite.hh"

@implementation SCStage {
    std::unique_ptr<sc::core::file_mapping<sc::sprites::atlas>> _mapper;
    std::unique_ptr<sc::render_bridge> _bridge;
    const sc::sprites::atlas* _atlas;
    sc::scene_population _registry;
    BOOL _keysPressed[128];
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.preferredFramesPerSecond = 120;

        self.delegate = self;

        self.drawableSize =
                CGSizeMake(sc::display::kWidth, sc::display::kHeight);
        self.layer.magnificationFilter = kCAFilterNearest;

        _mapper = std::make_unique<sc::core::file_mapping<sc::sprites::atlas>>(
                sc::assets::kCharacterAtlas);
        if (!(_mapper && *_mapper)) {
            NSLog(@"FATAL: Could not map sprite bank file.");
            abort();
        }

        _atlas = &(**_mapper);
        if (!_atlas->is_valid(_mapper->size())) {
            NSLog(@"FATAL: sprite bank header validation failed.");
            abort();
        }

        _bridge = std::make_unique<sc::render_bridge>(
                (__bridge MTL::Device*) device);
        _bridge->set_sprite_atlas(*_atlas);

        self.framebufferOnly = false;

        const auto id{sc::sprites::atlas_index::LANCIS};
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

- (void)drawInMTKView:(MTKView*)view
{
    float speed = 300.0f;
    _registry.dx[0] = _registry.dy[0] = 0;
    if (_keysPressed[13]) // W
        _registry.dy[0] -= speed;
    if (_keysPressed[1]) // S
        _registry.dy[0] += speed;
    if (_keysPressed[0]) // A
        _registry.dx[0] -= speed;
    if (_keysPressed[2]) // D
        _registry.dx[0] += speed;

    float deltaTime = 1.0f / view.preferredFramesPerSecond;
    _registry.update(
            *_atlas, deltaTime, sc::display::kWidth, sc::display::kHeight);

    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _bridge->begin_frame(drawable);
    _bridge->clear();
    _bridge->draw(_registry);
    _bridge->end_frame(drawable);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(NSEvent*)event
{
    [self.window makeFirstResponder:self];
}

- (void)keyDown:(NSEvent*)event
{
    _keysPressed[event.keyCode] = YES;
}

- (void)keyUp:(NSEvent*)event
{
    _keysPressed[event.keyCode] = NO;
}

@end
