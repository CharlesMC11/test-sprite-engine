#import <Cocoa/Cocoa.h>

#import "SCAppDelegate.hh"

int main(const int argc, const char* argv[])
{
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        SCAppDelegate* delegate = [[SCAppDelegate alloc] init];
        app.delegate = delegate;

        return NSApplicationMain(argc, argv);
    }
}
