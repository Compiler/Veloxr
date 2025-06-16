#import <Metal/Metal.h>
#include <string>

extern "C" {
    bool checkMetalAvailability() {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device == nil) {
            return false;
        }
        [device release];
        return true;
    }

    const char* getMetalDeviceName() {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device == nil) {
            return nullptr;
        }
        const char* name = [[device name] UTF8String];
        [device release];
        return name;
    }
} 