#pragma once
#include "Vertex.h"
#include "VLogger.h"


namespace Veloxr {

    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec4 roi;
        alignas(16) uint32_t hiddenMask;
        alignas(16) float nSplitVal;
    };

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
