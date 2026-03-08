#import <Cocoa/Cocoa.h>

#import "SCAppDelegate.h"


int main(const int argc, const char* argv[])
{
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        SCAppDelegate* delegate = [[SCAppDelegate alloc] init];
        app.delegate = delegate;
        return NSApplicationMain(argc, argv);
    }
}
