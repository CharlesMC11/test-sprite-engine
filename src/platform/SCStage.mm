#import "SCStage.h"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "definitions.hpp"
#include "entity_id.hpp"
#include "entity_layout.hpp"
#include "mapped_asset.hpp"
#include "render_bridge.hpp"
#include "sprite.hpp"
#include "sprite_bank.hpp"

@implementation SCStage {
    std::unique_ptr<sc::mapped_asset<sc::sprite_bank>> _loader;
    std::unique_ptr<sc::render_bridge> _bridge;
    const sc::sprite_bank* _bank;
    sc::entity_layout _layout;
    BOOL _keysPressed[128];
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.preferredFramesPerSecond = 120;

        self.delegate = self;

        self.drawableSize = CGSizeMake(sc::display::WIDTH, sc::display::HEIGHT);
        self.layer.magnificationFilter = kCAFilterNearest;

        _loader = std::make_unique<sc::mapped_asset<sc::sprite_bank>>(
                sc::assets::CHARACTER_SPRITE_BANK);
        if (!(_loader && *_loader)) {
            NSLog(@"FATAL: Could not map sprite bank file.");
            abort();
        }

        _bank = &(**_loader);
        if (!_bank->is_valid(_loader->size())) {
            NSLog(@"FATAL: sprite bank header validation failed.");
            abort();
        }

        _bridge = std::make_unique<sc::render_bridge>(
                (__bridge MTL::Device*) device);
        _bridge->set_sprite_bank(*_bank);

        self.framebufferOnly = false;

        const auto id{sc::entity_id::LANCIS};
        const sc::sprite& sprite{(*_bank)[id]};
        _layout.spawn(
                (sc::display::WIDTH - sc::SPRITE_WIDTH - sprite.anchor_x) *
                        0.5f,
                (sc::display::HEIGHT - sc::SPRITE_HEIGHT - sprite.anchor_y) *
                        0.5f,
                id);

        _layout.spawn(sc::display::HEIGHT * 0.75f, sc::display::HEIGHT * 0.75f,
                sc::entity_id::HEART);
    }

    return self;
}

- (void)drawInMTKView:(MTKView*)view
{
    float speed = 300.0f;
    _layout.dx[0] = _layout.dy[0] = 0;
    if (_keysPressed[13]) // W
        _layout.dy[0] -= speed;
    if (_keysPressed[1]) // S
        _layout.dy[0] += speed;
    if (_keysPressed[0]) // A
        _layout.dx[0] -= speed;
    if (_keysPressed[2]) // D
        _layout.dx[0] += speed;

    float deltaTime = 1.0f / view.preferredFramesPerSecond;
    _layout.update(*_bank, deltaTime, sc::display::WIDTH, sc::display::HEIGHT);

    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _bridge->begin_frame(drawable);
    _bridge->clear();
    _bridge->draw(_layout);
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
