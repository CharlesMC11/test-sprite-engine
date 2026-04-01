#import <Cocoa/Cocoa.h>

#import "SCAppDelegate.hh"

int main(const int argc, const char* argv[])
{
    @autoreleasepool {
        auto* app{[NSApplication sharedApplication]};
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        auto* delegate{[[SCAppDelegate alloc] init]};
        app.delegate = delegate;

        return NSApplicationMain(argc, argv);
    }
}
