#import "SCStage.h"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "atlas.hpp"
#include "memory_map.hpp"
#include "renderer.hpp"

@implementation SCStage {
    std::unique_ptr<sc::memory_map<sc::atlas>> _memory_map;
    const sc::atlas* _atlas;
    std::unique_ptr<sc::renderer> _renderer;
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.delegate = self;

        _memory_map = std::make_unique<sc::memory_map<sc::atlas>>(
                "data/master.atlas");
        if (!(_memory_map && *_memory_map)) {
            NSLog(@"FATAL: Could not map atlas file.");
            abort();
        }

        _atlas = &(**_memory_map);
        if (!_atlas->is_valid(_memory_map->size())) {
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
