#import "SCAppDelegate.hh"

#import <Cocoa/Cocoa.h>
#import <Metal/MTLDevice.h>
#import <MetalKit/MetalKit.h>

#include <memory>

#import "SCStage.hh"
#include "assets/asset_constants.hh"
#include "assets/atlas.hh"
#include "core/mapped_view.hh"
#include "graphics/display_constants.hh"
#include "simulation/world.hh"

@implementation SCAppDelegate {
    std::unique_ptr<sc::world> _world;
}

- (void)applicationDidFinishLaunching:(nonnull NSNotification*)notification
{
    constexpr double scale{4.0};
    const auto frame{NSMakeRect(0.0, 0.0, sc::display::kWidth * scale,
            sc::display::kHeight * scale)};
    self.window = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:(NSWindowStyleMaskTitled |
                                        NSWindowStyleMaskClosable |
                                        NSWindowStyleMaskResizable)
                        backing:NSBackingStoreBuffered
                          defer:NO

    ];
    [self.window setTitle:@"Test Sprite Engine"];
    [self.window center];

    id<MTLDevice> device{MTLCreateSystemDefaultDevice()};

    _world = std::make_unique<sc::world>((__bridge MTL::Device*) device);

    self.view = [[SCStage alloc] initWithFrame:frame
                                        device:device
                                         world:_world.get()];

    self.window.contentView = self.view;
    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
        (nonnull NSApplication*)sender
{
    return YES;
}

@end
