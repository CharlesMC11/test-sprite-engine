#import "SCStage.h"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "atlas.hpp"
#include "atlas_index.hpp"
#include "constants.hpp"
#include "memory_map.hpp"
#include "renderer.hpp"
#include "sprite.hpp"
#include "transform_registry.hpp"

@implementation SCStage {
    std::unique_ptr<sc::memory_map<sc::atlas>> _loader;
    const sc::atlas* _atlas;
    std::unique_ptr<sc::renderer> _renderer;
    sc::transform_registry _registry;
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.preferredFramesPerSecond = 120;

        self.delegate = self;

        self.drawableSize =
                CGSizeMake(sc::ui::SCREEN_WIDTH, sc::ui::SCREEN_HEIGHT);

        self.layer.magnificationFilter = kCAFilterNearest;

        _loader = std::make_unique<sc::memory_map<sc::atlas>>(
                sc::paths::CHARACTER_ATLAS);
        if (!(_loader && *_loader)) {
            NSLog(@"FATAL: Could not map atlas file.");
            abort();
        }

        _atlas = &(**_loader);
        if (!_atlas->is_valid(_loader->size())) {
            NSLog(@"FATAL: Atlas header validation failed.");
            abort();
        }

        _renderer =
                std::make_unique<sc::renderer>((__bridge MTL::Device*) device);

        self.framebufferOnly = false;

        _registry.add_entity(
                (sc::ui::SCREEN_WIDTH / 2) - (sc::SPRITE_WIDTH / 2),
                (sc::ui::SCREEN_HEIGHT / 2) - (sc::SPRITE_HEIGHT / 2),
                sc::atlas_index::LANCIS);

        _registry.add_entity(0, 0, sc::atlas_index::HEART);
    }

    return self;
}

- (void)drawInMTKView:(MTKView*)view
{
    float deltaTime = 1.0f / view.preferredFramesPerSecond;
    _registry.update(deltaTime, sc::ui::SCREEN_WIDTH, sc::ui::SCREEN_HEIGHT);

    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _renderer->begin_frame(drawable);

    _renderer->draw(_registry, *_atlas);

    _renderer->end_frame(drawable);
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
        _registry.dy[0] = -speed;
        break;
    case 0: // A
        _registry.dx[0] = -speed;
        break;
    case 1: // S
        _registry.dy[0] = speed;
        break;
    case 2: // D
        _registry.dx[0] = speed;
        break;
    }
}

- (void)keyUp:(NSEvent*)event
{
    switch (event.keyCode) {
    case 13:
    case 1:
        _registry.dy[0] = 0;
        break; // Stop Vertical
    case 0:
    case 2:
        _registry.dx[0] = 0;
        break; // Stop Horizontal
    }
}

@end
