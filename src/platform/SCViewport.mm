#import "SCViewport.h"

#import <MetalKit/MetalKit.h>

#include "atlas.hpp"
#include "memory_map.hpp"
#include "renderer.hpp"

@implementation SCViewport {
    sc::memory_map<sc::atlas>* _atlas;
    sc::renderer* _renderer;
}

- (instancetype)initWithFrame:(CGRect)frame device:(id<MTLDevice>)device
{
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.delegate = self;

        _atlas = new sc::memory_map<sc::atlas>("data/master.atlas");
        _renderer = new sc::renderer((__bridge MTL::Device*) device);

        self.framebufferOnly = false;
    }

    return self;
}

- (void)dealloc
{
    delete _renderer;
    delete _atlas;
    [super dealloc];
}

- (void)drawInMTKView:(MTKView*)view
{
    if (!_atlas)
        return;

    auto* rpd = (__bridge MTL::RenderPassDescriptor*)
                        view.currentRenderPassDescriptor;
    auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;

    const sc::sprite& player = (**_atlas)[sc::atlas_index::LANCIS];

    _renderer->draw(rpd, drawable, player);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}
@end
