#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

extern "C" void *GetMetalLayerForNSView(void *viewPtr) {
    NSView *nsView = (__bridge NSView *)viewPtr;

    if ([nsView.layer isKindOfClass:[CAMetalLayer class]]) {
        return (__bridge void *)nsView.layer;
    }

    [nsView setWantsLayer:YES];

    CAMetalLayer *metalLayer = [CAMetalLayer layer];
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.frame = nsView.bounds;
    metalLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    metalLayer.contentsScale = nsView.window.backingScaleFactor;
    metalLayer.drawableSize = CGSizeMake(nsView.bounds.size.width  * metalLayer.contentsScale, nsView.bounds.size.height * metalLayer.contentsScale);

    [nsView.layer insertSublayer:metalLayer atIndex:0];

    return (__bridge void *)metalLayer;
}

