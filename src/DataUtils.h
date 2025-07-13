#pragma once

#include <cstdint>
#include <vector>
namespace Veloxr{

    struct VeloxrBuffer {
        std::vector<unsigned char> data;
        uint64_t width, height, numChannels, orientation;
    };

}
