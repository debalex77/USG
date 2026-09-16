#include <Cocoa/Cocoa.h>

@interface CustomAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation CustomAppDelegate

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}

@end

void setupCustomAppDelegate() {
    CustomAppDelegate *delegate = [[CustomAppDelegate alloc] init];
    [NSApplication sharedApplication].delegate = delegate;
}

bool isDarkModeEnabledMacOS() {
    @autoreleasepool {
        NSString *appearanceName = [[NSApplication sharedApplication] effectiveAppearance].name;
        return [appearanceName containsString:@"Dark"];
    }
}
