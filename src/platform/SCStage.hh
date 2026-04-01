#ifndef SC_PLATFORM_STAGE_HH
#define SC_PLATFORM_STAGE_HH

#import <MetalKit/MetalKit.h>

#include "assets/atlas.hh"
#include "core/mapped_view.hh"

@interface SCStage : MTKView <MTKViewDelegate>

- (instancetype)initWithFrame:(CGRect)frameRect
                       device:(id<MTLDevice>)device
                  mappedAtlas:(const sc::core::mapped_view<sc::assets::atlas>*)
                                      atlas;

@end

#endif // SC_PLATFORM_STAGE_HH
