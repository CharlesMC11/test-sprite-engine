#pragma once

#import <Cocoa/Cocoa.h>

#import "SCViewport.h"

@interface SCAppDelegate : NSObject <NSApplicationDelegate>
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) SCViewport* view;
@end
