#ifndef SC_PLATFORM_APP_DELEGATE
#define SC_PLATFORM_APP_DELEGATE

#import <Cocoa/Cocoa.h>

#import "SCStage.hh"

@interface SCAppDelegate : NSObject <NSApplicationDelegate>
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) SCStage* view;
@end

#endif // SC_PLATFORM_APP_DELEGATE_HH
