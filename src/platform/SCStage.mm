#import "SCStage.h"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "atlas.hpp"
#include "constants.hpp"
#include "memory_map.hpp"
#include "renderer.hpp"

@implementation SCStage {
    std::unique_ptr<sc::memory_map<sc::atlas>> _loader;
    const sc::atlas* _atlas;
    std::unique_ptr<sc::renderer> _renderer;
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
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
    }

    return self;
}

- (void)drawInMTKView:(MTKView*)view
{
    const auto* rpd = (__bridge MTL::RenderPassDescriptor*)
                              view.currentRenderPassDescriptor;
    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;

    const sc::sprite& player = (*_atlas)[sc::atlas_index::LANCIS];
    _renderer->draw(rpd, drawable, player);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}
@end
