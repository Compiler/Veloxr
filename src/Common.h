#pragma once
#include "Vertex.h"
#include "VLogger.h"

namespace Veloxr {

    struct VVDataPacket {
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkCommandPool commandPool;
        VkQueue graphicsQueue, presentQueue;
    };

    typedef uint64_t v_int;

    struct Point {
        uint64_t x, y;
    };

    enum EXIFCases {
        CW_0  =1,
        CW_180=3,
        CW_90 =6,
        CW_270=8
    };

}
