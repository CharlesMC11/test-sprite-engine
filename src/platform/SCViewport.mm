#import "SCViewport.h"

#import <MetalKit/MetalKit.h>

#include <memory>

#include "atlas.hpp"
#include "memory_map.hpp"
#include "renderer.hpp"

@implementation SCViewport {
    std::unique_ptr<sc::memory_map<sc::atlas>> _atlas;
    std::unique_ptr<sc::renderer> _renderer;
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.delegate = self;

        _atlas = std::make_unique<sc::memory_map<sc::atlas>>(
                "data/master.atlas");
        _renderer =
                std::make_unique<sc::renderer>((__bridge MTL::Device*) device);

        self.framebufferOnly = false;
    }

    return self;
}

- (void)drawInMTKView:(MTKView*)view
{
    if (!_atlas || !(*_atlas)->is_valid(_atlas->size()))
        return;

    const auto* rpd = (__bridge MTL::RenderPassDescriptor*)
                              view.currentRenderPassDescriptor;
    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;

    const sc::sprite& player = (**_atlas)[sc::atlas_index::LANCIS];
    _renderer->draw(rpd, drawable, player);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}
@end
