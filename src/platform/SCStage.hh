#ifndef SC_PLATFORM_STAGE_HH
#define SC_PLATFORM_STAGE_HH

#import <MetalKit/MetalKit.h>

#include "simulation/world.hh"

@interface SCStage : MTKView <MTKViewDelegate>

- (instancetype)initWithFrame:(CGRect)frameRect
                       device:(id<MTLDevice>)device
                        world:(sc::world*)world;

@end

#endif // SC_PLATFORM_STAGE_HH
