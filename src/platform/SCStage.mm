#import "SCStage.h"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "atlas.hpp"
#include "atlas_index.hpp"
#include "constants.hpp"
#include "memory_map.hpp"
#include "renderer.hpp"
#include "transform_registry.h"

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

        _registry.x.reserve(8);
        _registry.y.reserve(8);
        _registry.vx.reserve(8);
        _registry.vy.reserve(8);
        _registry.sprite_ids.reserve(8);

        _registry.add_entity(sc::ui::SCREEN_WIDTH / 2,
                sc::ui::SCREEN_HEIGHT / 2, sc::atlas_index::LANCIS);

        _registry.add_entity(sc::ui::SCREEN_WIDTH * 3 / 4,
                sc::ui::SCREEN_HEIGHT * 3 / 4, sc::atlas_index::HEART);
    }

    return self;
}

- (void)drawInMTKView:(MTKView*)view
{
    float deltaTime = 1.0f / view.preferredFramesPerSecond;

    _registry.update(deltaTime);

    const auto* rpd = (__bridge MTL::RenderPassDescriptor*)
                              view.currentRenderPassDescriptor;
    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;

    for (std::size_t i{0}; i < _registry.size(); ++i) {
        const sc::sprite& sprite = (*_atlas)[_registry.sprite_ids[i]];
        _renderer->draw(rpd, drawable, sprite, _registry.x[i], _registry.y[i]);
    }
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
        _registry.vy[0] = -speed;
        break;
    case 0: // A
        _registry.vx[0] = -speed;
        break;
    case 1: // S
        _registry.vy[0] = speed;
        break;
    case 2: // D
        _registry.vx[0] = speed;
        break;
    }
}

- (void)keyUp:(NSEvent*)event
{
    switch (event.keyCode) {
    case 13:
    case 1:
        _registry.vy[0] = 0;
        break; // Stop Vertical
    case 0:
    case 2:
        _registry.vx[0] = 0;
        break; // Stop Horizontal
    }
}

@end
