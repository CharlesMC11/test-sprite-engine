#pragma once

#import <Cocoa/Cocoa.h>

#import "SCStage.h"

@interface SCAppDelegate : NSObject <NSApplicationDelegate>
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) SCStage* view;
@end
