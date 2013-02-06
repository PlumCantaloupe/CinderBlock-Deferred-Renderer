//
//  Something.h
//  CinderDeferredRendering
//
//  Created by Anthony Scavarelli on 2013-02-06.
//
//

#include    "AppleUtilities.h"

@interface CocoaProxy : NSObject
{}
+ (void)autohideMenu;
@end

@implementation CocoaProxy

+ (void)autohideMenu {
    //look in NSApplication.h for more options such as NSApplicationPresentationHideDock and NSApplicationPresentationHideMenuBar
    [[NSApplication sharedApplication] setPresentationOptions:NSApplicationPresentationAutoHideMenuBar|NSApplicationPresentationAutoHideDock];
}

@end

void AppleUtilities::autohideMenuBar(){
    [CocoaProxy autohideMenu];
}
