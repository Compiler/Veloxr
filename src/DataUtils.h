#pragma once

#include <vector>
namespace Veloxr{

    struct VeloxrBuffer {
        std::vector<unsigned char> data;
        int width, height, numChannels, orientation;
    };

}
