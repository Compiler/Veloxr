#pragma once
#include "DataUtils.h"
#include <cstring>
#include <iostream>
#include <map>
#include <texture.h>
#include <vector>
#include <Vertex.h>
#include "VLogger.h"

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagecache.h>
#include <OpenImageIO/ustring.h>
#include <OpenImageIO/imagebuf.h>
namespace Veloxr {

    struct TextureData {
        uint32_t width, height, channels;
        std::vector<unsigned char> pixelData;
        uint32_t rotateIndex=0;
        uint32_t samplerIndex{};
    };

    struct TiledResult {
        std::map<int, TextureData> tiles;
        std::vector<Vertex>      vertices;
        glm::vec4 boundingBox;
    };
    class MemoryRawInput : public OIIO::ImageInput {
        public:
            MemoryRawInput(const void* pixel_data, const OIIO::ImageSpec& spec)
                : m_pixel_data(pixel_data), m_spec(spec) {}

            const char* format_name(void) const override { return "memory_raw"; }

            bool open(const std::string& name, OIIO::ImageSpec& newspec) override {
                newspec = m_spec;
                return true;
            }

            bool close(void) override { return true; }

            bool read_native_scanline(int subimage, int miplevel, int y, int z, void* data) override {
                if (subimage != current_subimage() || miplevel != current_miplevel() || z != 0)
                    return false;
                size_t scanline_size = m_spec.scanline_bytes();
                const char* src = static_cast<const char*>(m_pixel_data) + static_cast<size_t>(y) * scanline_size;
                memcpy(data, src, scanline_size);
                return true;
            }

        private:
            const void* m_pixel_data;
            OIIO::ImageSpec m_spec;
    };


    class TextureTiling {
        private:
            Veloxr::LLogger console{"[Veloxr] [TextureTiling] "};
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
            TiledResult tile8(Veloxr::VeloxrBuffer& buffer, uint32_t maxResolution=4096*2);


    };

}
