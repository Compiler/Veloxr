#pragma once
#include <texture.h>

#include <vector>
namespace Veloxr {

    struct TextureData {
        uint32_t width, height, channels;
        std::vector<unsigned char> pixelData;
    };

    class TextureTiling {
        
        public:
            TextureTiling() = default;
            std::vector<TextureData> tile(OIIOTexture& texture, uint32_t maxResolution = 4096*2);

    };

}
