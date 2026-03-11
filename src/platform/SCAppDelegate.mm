#import "SCAppDelegate.hh"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "SCStage.hh"
#include "core.hh"

@implementation SCAppDelegate

- (void)applicationDidFinishLaunching:(nonnull NSNotification*)notification
{
    constexpr float scale{4.0f};
    const auto frame{NSMakeRect(
            0, 0, sc::display::kWidth * scale, sc::display::kHeight * scale)};
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
    self.view = [[SCStage alloc] initWithFrame:frame device:device];

    self.window.contentView = self.view;
    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
        (nonnull NSApplication*)sender
{
    return YES;
}

@end
