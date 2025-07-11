#import <Cocoa/Cocoa.h>
#include <iostream>
#import <QuartzCore/CAMetalLayer.h>

extern "C" void *GetMetalLayerForNSView(void *viewPtr) {
    NSView *nsView = (__bridge NSView *)viewPtr;
    std::cout << "[Veloxr] GetMetalLayerForNSView nsView=" << nsView << std::endl;

    if ([nsView.layer isKindOfClass:[CAMetalLayer class]]) {
        CAMetalLayer *metal = (CAMetalLayer *)nsView.layer;

        // If Qt hasn’t set the drawable size yet, do it now
        if (metal.drawableSize.width == 0 || metal.drawableSize.height == 0) {
            metal.contentsScale = nsView.window.backingScaleFactor;
            metal.drawableSize  = CGSizeMake(nsView.bounds.size.width  * metal.contentsScale,
                                             nsView.bounds.size.height * metal.contentsScale);
            std::cout << "[Veloxr] Fixed drawableSize → (" << metal.drawableSize.width
                      << "," << metal.drawableSize.height << ")" << std::endl;
        } else {
            std::cout << "[Veloxr] Reusing drawableSize=(" << metal.drawableSize.width
                      << "," << metal.drawableSize.height << ")" << std::endl;
        }
        return (__bridge void *)metal;
    }

/*
    if ([nsView.layer isKindOfClass:[CAMetalLayer class]]) {
        CAMetalLayer *existing = (CAMetalLayer *)nsView.layer;
        std::cout << "[Veloxr] Reusing existing CAMetalLayer=" << existing
                  << " drawableSize=(" << existing.drawableSize.width << ","
                  << existing.drawableSize.height << ")" << std::endl;
        return (__bridge void *)existing;
    }
    */

    [nsView setWantsLayer:YES];

    CAMetalLayer *metalLayer = [CAMetalLayer layer];
    metalLayer.pixelFormat      = MTLPixelFormatBGRA8Unorm;
    metalLayer.frame            = nsView.bounds;
    metalLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    metalLayer.contentsScale    = nsView.window.backingScaleFactor;
    metalLayer.drawableSize     = CGSizeMake(nsView.bounds.size.width  * metalLayer.contentsScale,
                                             nsView.bounds.size.height * metalLayer.contentsScale);

    [nsView.layer insertSublayer:metalLayer atIndex:0];

    std::cout << "[Veloxr] Created CAMetalLayer=" << metalLayer
              << " drawableSize=(" << metalLayer.drawableSize.width << ","
              << metalLayer.drawableSize.height << ") scale=" << metalLayer.contentsScale << std::endl;

    return (__bridge void *)metalLayer;
}


