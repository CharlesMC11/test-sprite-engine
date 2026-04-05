#import "SCStage.hh"

#import <MetalKit/MetalKit.h>

#include "core/input.hh"
#include "simulation/world.hh"

@implementation SCStage {
    sc::world* _world;
    float _accumulator;
    sc::input::mask _keysPressed;
}

- (instancetype)initWithFrame:(CGRect)frameRect
                       device:(id<MTLDevice>)device
                        world:(sc::world*)world
{
    self = [super initWithFrame:frameRect device:device];
    if (self) {
        self.delegate = self;
        self.drawableSize =
                CGSizeMake(sc::display::kWidth, sc::display::kHeight);
        self.preferredFramesPerSecond = (NSInteger) sc::display::kTargetFPS;

        self.layer.magnificationFilter = kCAFilterNearest;

        self.framebufferOnly = false;

        _world = world;
    }

    return self;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(nonnull NSEvent*)event
{
    [self.window makeFirstResponder:self];
}

- (void)keyDown:(nonnull NSEvent*)event
{
    switch (event.keyCode) {
    case 13U:
    case 126U:
        _keysPressed |= sc::input::mask::up;
        break;
    case 1U:
    case 125U:
        _keysPressed |= sc::input::mask::down;
        break;
    case 0U:
    case 123U:
        _keysPressed |= sc::input::mask::left;
        break;
    case 2U:
    case 124U:
        _keysPressed |= sc::input::mask::right;
        break;
    default:
        break;
    }
}

- (void)keyUp:(nonnull NSEvent*)event
{
    switch (event.keyCode) {
    case 13U:
    case 126U:
        _keysPressed &= ~sc::input::mask::up;
        break;
    case 1U:
    case 125U:
        _keysPressed &= ~sc::input::mask::down;
        break;
    case 0U:
    case 123U:
        _keysPressed &= ~sc::input::mask::left;
        break;
    case 2U:
    case 124U:
        _keysPressed &= ~sc::input::mask::right;
        break;
    default:
        break;
    }
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

- (void)drawInMTKView:(nonnull MTKView*)view
{
    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _world->update(_keysPressed, drawable);
}

@end
