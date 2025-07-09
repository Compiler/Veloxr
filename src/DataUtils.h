#pragma once

#include <cstdint>
#include <vector>

namespace Veloxr{

    struct VeloxrBuffer {
        std::vector<unsigned char> data;
        int width, height, numChannels, orientation;
    };

    struct TextureBuffer {
        unsigned char* data;
        uint32_t lengthOfData;
        uint32_t width, height, numChannels, orientation;
    };

}
