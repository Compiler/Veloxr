#pragma once
#include <cstring>
#include <iostream>
#include <map>
#include <texture.h>
#include <vector>
#include <Vertex.h>
namespace Veloxr {

    struct TextureData {
        uint32_t width, height, channels;
        std::vector<unsigned char> pixelData;
        uint32_t rotateIndex=0;
    };

    struct TiledResult {
        std::map<int, TextureData> tiles;
        std::vector<Vertex>      vertices;
    };


    class TextureTiling {

        public:
            TextureTiling() = default;
            void init();
            std::vector<TextureData> tile(OIIOTexture& texture, uint32_t maxResolution = 4096*2);

            TiledResult tile2(OIIOTexture &texture, uint32_t maxResolution=4096*2);
            TiledResult tile3(OIIOTexture &texture, uint32_t maxResolution=4096*2);
            TiledResult tile4(OIIOTexture &texture, uint32_t maxResolution=4096*2);
            TiledResult tile5(OIIOTexture &texture, uint32_t maxResolution=4096*2);
            TiledResult tile6(OIIOTexture &texture, uint32_t maxResolution=4096*2);
            TiledResult tile7(OIIOTexture &texture, uint32_t maxResolution=4096*2);
            TiledResult tile8(OIIOTexture &texture, uint32_t maxResolution=4096*2);


    };

}
