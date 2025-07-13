#pragma once
#include "DataUtils.h"
#include <cstring>
#include <iostream>
#include <map>
#include <texture.h>
#include <vector>
#include "Common.h"
#include "VLogger.h"
namespace Veloxr {


    struct TextureData {
        uint32_t width, height, channels;
        std::vector<unsigned char> pixelData;
        uint32_t rotateIndex=0;
        uint32_t samplerIndex{};
    };

    struct TiledResult {
        std::map<int, TextureData> tiles;
        std::vector<Vertex> vertices;
        glm::vec4 boundingBox;
    };


    class TextureTiling {
        private:
            Veloxr::LLogger console {"[Veloxr][TextureTiling] "};

            // Helpers for tile()
            glm::vec2 rotatePositionForOrientation(const glm::vec2 &p, int orientation, float width, float height);
            void applyExifOrientation( std::vector<Vertex>& vertices, int orientation, uint32_t rawW, uint32_t rawH) ;


        public:
            TextureTiling() = default;
            void init();
            TiledResult tile(OIIOTexture &texture, uint32_t maxResolution=4096*2);
            TiledResult tile(Veloxr::VeloxrBuffer& buffer, uint32_t maxResolution=4096*2);


    };

}
