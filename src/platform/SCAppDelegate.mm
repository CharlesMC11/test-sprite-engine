#import "SCAppDelegate.hh"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include <memory>

#import "SCStage.hh"
#include "assets/asset_constants.hh"
#include "assets/atlas.hh"
#include "core/mapped_view.hh"
#include "graphics/display_constants.hh"

@implementation SCAppDelegate {
    std::unique_ptr<sc::core::mapped_view<sc::assets::atlas>> _mappedAtlas;
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

    _mappedAtlas = std::make_unique<sc::core::mapped_view<sc::assets::atlas>>(
            sc::assets::kAtlas);
    if (!(_mappedAtlas && *_mappedAtlas)) {
        NSLog(@"FATAL: Could not map atlas file!");
        abort();
    }
    if (!sc::assets::atlas::validate(
                _mappedAtlas->data(), _mappedAtlas->size())) {
        NSLog(@"FATAL: Atlas has an invalid header!");
        abort();
    }

    self.view = [[SCStage alloc] initWithFrame:frame
                                        device:device
                                   mappedAtlas:_mappedAtlas.get()];

    self.window.contentView = self.view;
    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
        (nonnull NSApplication*)sender
{
    return YES;
}

@end
