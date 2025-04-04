#pragma once
#include <texture.h>

#include <vector>
namespace Veloxr {

    class TextureTiling {
        
        public:
            TextureTiling() = default;
            std::vector<std::vector<unsigned char>> tile(OIIOTexture& texture, uint32_t maxResolution = 4096*2);

    };

}
