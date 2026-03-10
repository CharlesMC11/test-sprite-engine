#import "SCAppDelegate.h"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "SCStage.h"
#include "constants.hpp"

@implementation SCAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    const auto scale = 4.0f;
    auto frame = NSMakeRect(
            0, 0, sc::display::WIDTH * scale, sc::display::HEIGHT * scale);
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

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    self.view = [[SCStage alloc] initWithFrame:frame device:device];
    self.window.contentView = self.view;

    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}
@end
