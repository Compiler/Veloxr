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
        glm::vec4 boundingBox;
    };


    class TextureTiling {
        private:
            inline glm::vec2 rotatePositionForOrientation(const glm::vec2 &p, int orientation, float width, float height) {
                switch (orientation) {
                    case 1: 
                    default:return p;
                    case 3: return {width - p.x, height - p.y};
                    case 6: return {p.y, width - p.x};
                    case 8: return {height - p.y, p.x};
                }
            }
            // Rotate & remap all vertices in-place based on EXIF orientation.
            void applyExifOrientation( std::vector<Vertex>& vertices, int orientation, uint32_t rawW, uint32_t rawH) {
                for (auto& v : vertices) {
                    float oldX = v.pos.x;
                    float oldY = v.pos.y;

                    float newX = oldX;
                    float newY = oldY;

                    switch (orientation) {
                        case 1: break;
                        case 3: // 180 degrees
                            newX = float(rawW)  - oldX;
                            newY = float(rawH)  - oldY;
                            break;

                        case 6: // 90 CW
                            newX = oldY;
                            newY = float(rawW) - oldX;
                            break;

                        case 8: // 270 CW
                            newX = float(rawH) - oldY;
                            newY = oldX;
                            break;

                        default: // TODO: orientation=2,4,5,7 are mirrored variants
                            break;
                    }

                    v.pos.x = newX;
                    v.pos.y = newY;

                    float u = v.texCoord.x;
                    float w = v.texCoord.y;

                    glm::vec2 uvRes(u, w);
                    switch (orientation) {
                        case 1:
                            break;
                        case 3:
                            uvRes = glm::vec2(1.0f - w, u);
                            break;
                        case 6:
                            uvRes = glm::vec2(w, 1.0f - u);
                            break;
                        case 8:
                            uvRes = glm::vec2(1.0f - u, 1.0f - w);
                            break;
                        default:
                            break;
                    }

                    //v.texCoord.x = uvRes.x;
                    //v.texCoord.y = uvRes.y;
                }
            }


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
