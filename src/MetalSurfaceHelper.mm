#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

extern "C" void* GetMetalLayerForNSView(void* viewPtr)
{
    NSView* nsView = (__bridge NSView*)viewPtr;
    if (!nsView.layer || ![nsView.layer isKindOfClass:[CAMetalLayer class]]) {
        [nsView setWantsLayer:YES];
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        [nsView setLayer:metalLayer];
    }
    return (__bridge void*)nsView.layer; 
}

