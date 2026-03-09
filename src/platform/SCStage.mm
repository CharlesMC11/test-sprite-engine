#import "SCStage.h"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "constants.hpp"
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
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.preferredFramesPerSecond = 120;

        self.delegate = self;

        self.drawableSize = CGSizeMake(
                sc::display::SCREEN_WIDTH, sc::display::SCREEN_HEIGHT);

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

        self.framebufferOnly = false;

        const auto id{sc::entity_id::LANCIS};
        const sc::sprite& sprite{(*_bank)[id]};
        _layout.spawn((sc::display::SCREEN_WIDTH - sc::SPRITE_WIDTH - sprite.anchor_x) * 0.5f,
                (sc::display::SCREEN_HEIGHT - sc::SPRITE_HEIGHT - sprite.anchor_y) * 0.5f, id);

        _layout.spawn(0, 0, sc::entity_id::HEART);
    }

    return self;
}

- (void)drawInMTKView:(MTKView*)view
{
    float deltaTime = 1.0f / view.preferredFramesPerSecond;
    _layout.update(
            deltaTime, sc::display::SCREEN_WIDTH, sc::display::SCREEN_HEIGHT);

    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _renderer->begin_frame(drawable);

    _renderer->draw(_registry, *_atlas);

    _renderer->end_frame(drawable);
    _bridge->begin_frame(drawable);
    _bridge->draw(*_bank, _layout);
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
    const float speed{200.0f}; // Pixels per second

    switch (event.keyCode) {
    case 13: // W
        _layout.dy[0] = -speed;
        break;
    case 0: // A
        _layout.dx[0] = -speed;
        break;
    case 1: // S
        _layout.dy[0] = speed;
        break;
    case 2: // D
        _layout.dx[0] = speed;
        break;
    }
}

- (void)keyUp:(NSEvent*)event
{
    switch (event.keyCode) {
    case 13: // W
    case 1: // A
        _layout.dy[0] = 0;
        break; // Stop Vertical
    case 0: // S
    case 2: // D
        _layout.dx[0] = 0;
        break; // Stop Horizontal
    }
}

@end
