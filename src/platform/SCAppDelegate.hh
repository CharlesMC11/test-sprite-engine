#pragma once

#import <Cocoa/Cocoa.h>

#import "SCStage.hh"

@interface SCAppDelegate : NSObject <NSApplicationDelegate>
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) SCStage* view;
@end
