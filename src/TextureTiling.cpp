#include "TextureTiling.h"
#include <cmath>
#include <iostream>
#include <set>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

#include <thread>


#include <limits>


#include <limits>

namespace Veloxr {

    class MemoryRawInput : public OIIO::ImageInput {
        public:
            MemoryRawInput(const void* pixel_data, const OIIO::ImageSpec& spec)
                : m_pixel_data(pixel_data), m_spec(spec) {}

            const char* format_name(void) const override { return "memory_raw"; }

            bool open(const std::string& name, OIIO::ImageSpec& newspec) override {
                m_current_subimage = 0;
                m_current_miplevel = 0;
                newspec = m_spec;
                return true;
            }

            bool close(void) override { return true; }

            bool read_native_scanline(int subimage, int miplevel, int y, int z, void* data) override {
                if (subimage != m_current_subimage || miplevel != m_current_miplevel || z != 0)
                    return false;
                size_t scanline_size = m_spec.scanline_bytes();
                const char* src = static_cast<const char*>(m_pixel_data) + static_cast<size_t>(y) * scanline_size;
                memcpy(data, src, scanline_size);
                return true;
            }

        private:
            const void* m_pixel_data;
            OIIO::ImageSpec m_spec;
            int m_current_subimage{};
            int m_current_miplevel{};
    };

    namespace {
        const void* g_pixel_data = nullptr;
        OIIO::ImageSpec g_cache_spec;
        OIIO::ImageInput* create_memory_input() {
            return new MemoryRawInput(g_pixel_data, g_cache_spec);
        }
    }

} // namespace Veloxr

TiledResult TextureTiling::tile8(Veloxr::VeloxrBuffer& buffer, uint32_t deviceMaxDimension) {
    TiledResult result;
    if (buffer.data.empty()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return result;
    }

    uint32_t w = buffer.width;
    uint32_t h = buffer.height;
    console.logc2("Data size: ", buffer.data.size(), " - expected: ", (size_t)w * h * buffer.numChannels );

    uint64_t maxPixels   = (uint64_t)deviceMaxDimension * (uint64_t)deviceMaxDimension;
    uint64_t totalPixels = (uint64_t)w * (uint64_t)h;

    bool tooManyPixels = (totalPixels > maxPixels);
    bool tooWide       = (w > deviceMaxDimension);
    bool tooTall       = (h > deviceMaxDimension);

    std::cout << "[Veloxr]" << "MaxPixels=" << maxPixels
              << " totalPixels=" << totalPixels
              << " deviceMaxDimension=" << deviceMaxDimension << "\n";
    std::cout << "[Veloxr]" << "Resolution (raw) = " << w << " x " << h
              << " Channels=" << buffer.numChannels << "\n";
    std::cout << "[Veloxr]" << "tooManyPixels=" << tooManyPixels
              << " tooWide=" << tooWide
              << " tooTall=" << tooTall << "\n";

    result.boundingBox.x = std::numeric_limits<float>::max();
    result.boundingBox.y = std::numeric_limits<float>::max();
    result.boundingBox.z = std::numeric_limits<float>::lowest();
    result.boundingBox.w = std::numeric_limits<float>::lowest();

    if (!tooManyPixels && !tooWide && !tooTall) {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4; // forcing RGBA

        if (buffer.numChannels != 4) {
            std::vector<unsigned char> pixelData((size_t)w * h * 4, 255);
            for (uint32_t i = 0; i < w * h; ++i) {
                for (int c = 0; c < buffer.numChannels && c < 4; ++c) {
                    pixelData[i * 4 + c] = buffer.data[i * buffer.numChannels + c];
                }
            }
            one.pixelData = std::move(pixelData);
        } else {
            one.pixelData = buffer.data;
        }

        std::cout << "[Veloxr]" << "Loaded pixelData.size()=" << one.pixelData.size() << "\n";

        int orientation = buffer.orientation;
        uint32_t orientedW = w;
        uint32_t orientedH = h;
        if (orientation == 6 || orientation == 8) {
            std::swap(orientedW, orientedH);
        }

        float left   = 0.0f;
        float right  = float(orientedW);
        float top    = 0.0f;
        float bottom = float(orientedH);
        int idx      = 0;
        one.samplerIndex = idx;

        std::vector<Vertex> singleTileVerts = {
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { left,  bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, top,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx },
        };

        for (auto &v : singleTileVerts) {
            glm::vec2 uv(v.texCoord.x, v.texCoord.y);

            glm::vec2 res;

            switch (orientation) {
                case 1:
                    std::cout << "[Veloxr]" << "Tile has no orientation change.\n";
                    res = uv; 
                    break;
                case 3:
                    std::cout << "[Veloxr]" << "Tile has 180 rotation.\n";
                    res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                    break;
                case 6:
                    std::cout << "[Veloxr]" << "Tile has 90 rotation.\n";
                    res = glm::vec2(uv.y, 1.0f - uv.x);
                    break;
                case 8:
                    std::cout << "[Veloxr]" << "Tile has 270 rotation.\n";
                    res = glm::vec2(1.0f - uv.y, uv.x);
                    break;
                default:
                    res = uv;
                    break;
            }
            v.texCoord.x = res.x;
            v.texCoord.y = res.y;
        }

        result.tiles[0] = one;
        result.vertices.insert(result.vertices.end(),
                               singleTileVerts.begin(),
                               singleTileVerts.end());

        std::cout << "[Veloxr]" << "Single-tile approach used. \n";
        std::cout << "[Veloxr]" << "Tile " << idx
                  << " (" << one.width << " x " << one.height << ") completed\n";

        for(const auto& v : result.vertices) {
            result.boundingBox.x = std::min(v.pos.x, result.boundingBox.x);
            result.boundingBox.y = std::min(v.pos.y, result.boundingBox.y);
            result.boundingBox.z = std::max(v.pos.x, result.boundingBox.z);
            result.boundingBox.w = std::max(v.pos.y, result.boundingBox.w);
        }

        return result;
    }

    std::cout << "[Veloxr]" << "Texture too big for single tile. Doing multi-tiling.\n";

    uint32_t rawW = w; 
    uint32_t rawH = h;

    int orientation = buffer.orientation;
    std::cout << "[Veloxr]" << "[INFO] Orientation = " << orientation << "\n";

    uint32_t orientedW = rawW;
    uint32_t orientedH = rawH;
    if (orientation == 6 || orientation == 8) {
        std::swap(orientedW, orientedH);
    }

    uint32_t Nx = (rawW + deviceMaxDimension - 1) / deviceMaxDimension;
    uint32_t Ny = (rawH + deviceMaxDimension - 1) / deviceMaxDimension;

    uint32_t tileW = (rawW + Nx - 1) / Nx;
    uint32_t tileH = (rawH + Ny - 1) / Ny;

    int totalTiles = Nx * Ny;
    int numThreads = std::min(totalTiles, 4);  // Reduced to 4 to lower concurrent memory usage

    int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;

    std::shared_ptr<OIIO::ImageCache> ic = OIIO::ImageCache::create(true);
    ic->attribute("max_memory_MB", 1024.0f);

    g_pixel_data = buffer.data.data();
    g_cache_spec = OIIO::ImageSpec(rawW, rawH, buffer.numChannels, OIIO::TypeDesc::UINT8);

    OIIO::ImageInput::Creator creator = create_memory_input;

    std::string fake_filename = "memory_buffer";
    ic->add_file(OIIO::ustring(fake_filename), creator);

    OIIO::ImageSpec const *mainSpec = ic->imagespec(OIIO::ustring(fake_filename), 0, 0);
    if (!mainSpec) {
        std::cerr << "[TILER] Could not get ImageSpec from ImageCache for memory buffer\n";
        OIIO::ImageCache::destroy(ic);
        return result;
    }

    uint32_t originalChannels = mainSpec->nchannels;
    uint32_t forcedChannels   = 4;

    struct ThreadResult {
        std::map<int, TextureData>         localTiles;
        std::map<int, std::vector<Vertex>> localVerts;
    };
    std::vector<ThreadResult> partialResults(numThreads);

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int t = 0; t < numThreads; t++) {
        int startIdx = t * tilesPerThread;
        int endIdx   = std::min(totalTiles, startIdx + tilesPerThread);

        threads.emplace_back([=, &partialResults, &ic]() {
            auto &localTiles = partialResults[t].localTiles;
            auto &localVerts = partialResults[t].localVerts;

            for (int idx = startIdx; idx < endIdx; idx++) {
                int row = idx / Nx;
                int col = idx % Nx;

                uint32_t x0 = col * tileW;
                uint32_t x1 = std::min(x0 + tileW, rawW);
                uint32_t y0 = row * tileH;
                uint32_t y1 = std::min(y0 + tileH, rawH);

                uint32_t thisTileW = (x1 > x0) ? (x1 - x0) : 0;
                uint32_t thisTileH = (y1 > y0) ? (y1 - y0) : 0;
                if (!thisTileW || !thisTileH) {
                    continue;
                }

                std::vector<unsigned char> readBuffer(size_t(thisTileW) * size_t(thisTileH) * size_t(originalChannels), 0);

                bool ok = ic->get_pixels(
                    OIIO::ustring(fake_filename),   
                    0 /*subimage*/, 
                    0 /*miplevel*/,
                    (int)x0, (int)x1,
                    (int)y0, (int)y1,
                    0, 1,  // z range
                    0, (int)originalChannels,  // channel range
                    OIIO::TypeDesc::UINT8,
                    readBuffer.data()          // destination
                );
                if (!ok) {
                    std::cerr << "[TILER] Error reading tile " << idx
                              << " in thread " << t << ": "
                              << ic->geterror() << "\n";
                    continue;
                }

                std::vector<unsigned char> tileData(
                    size_t(thisTileW) * size_t(thisTileH) * size_t(forcedChannels),
                    255
                );

                for (uint32_t yy = 0; yy < thisTileH; yy++) {
                    size_t srcRowOffset = size_t(yy) * size_t(thisTileW) * size_t(originalChannels);
                    size_t dstRowOffset = size_t(yy) * size_t(thisTileW) * size_t(forcedChannels);
                    for (uint32_t xx = 0; xx < thisTileW; xx++) {
                        size_t srcPix = srcRowOffset + size_t(xx) * size_t(originalChannels);
                        size_t dstPix = dstRowOffset + size_t(xx) * size_t(forcedChannels);

                        for (uint32_t c = 0; c < forcedChannels; c++) {
                            if (c < originalChannels) {
                                tileData[dstPix + c] = readBuffer[srcPix + c];
                            } else {
                                tileData[dstPix + c] = 255;
                            }
                        }
                    }
                }

                TextureData data;
                data.width     = thisTileW;
                data.height    = thisTileH;
                data.channels  = forcedChannels;
                data.pixelData = std::move(tileData);
                data.samplerIndex = idx;
                localTiles[idx] = std::move(data);

                float tileLeft   = (float(x0));
                float tileRight  = (float(x1));
                float tileTop    = (float(y0));
                float tileBottom = (float(y1));

                Vertex v0 = { { tileLeft,  tileTop,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v1 = { { tileLeft,  tileBottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v2 = { { tileRight, tileBottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v3 = { { tileLeft,  tileTop,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v4 = { { tileRight, tileBottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v5 = { { tileRight, tileTop,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx };

                std::vector<Vertex> theseVerts = { v0, v1, v2, v3, v4, v5 };
                localVerts[idx] = std::move(theseVerts);

                std::cout << "[Veloxr]" << "Tile " << idx << " (thread " << t << ") completed.\n";
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    std::set<int> allIndices;
    for (int t = 0; t < numThreads; t++) {
        for (auto &kv : partialResults[t].localTiles) {
            allIndices.insert(kv.first);
        }
    }
    for (int idx : allIndices) {
        for (int t = 0; t < numThreads; t++) {
            auto itTiles = partialResults[t].localTiles.find(idx);
            if (itTiles != partialResults[t].localTiles.end()) {
                result.tiles[idx] = std::move(itTiles->second);
            }
            auto itVerts = partialResults[t].localVerts.find(idx);
            if (itVerts != partialResults[t].localVerts.end()) {
                auto &verts = itVerts->second;
                result.vertices.insert(result.vertices.end(),
                                       verts.begin(),
                                       verts.end());
            }
        }
    }

    applyExifOrientation(result.vertices, orientation, w, h );

    for(const auto& v : result.vertices) {
        result.boundingBox.x = std::min(v.pos.x, result.boundingBox.x);
        result.boundingBox.y = std::min(v.pos.y, result.boundingBox.y);
        result.boundingBox.z = std::max(v.pos.x, result.boundingBox.z);
        result.boundingBox.w = std::max(v.pos.y, result.boundingBox.w);
    }

    OIIO::ImageCache::destroy(ic);

    return result;
}

TiledResult TextureTiling::tile8(OIIOTexture &texture, uint32_t deviceMaxDimension) {
    TiledResult result;
    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return result;
    }

    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;

    uint64_t maxPixels   = (uint64_t)deviceMaxDimension * (uint64_t)deviceMaxDimension;
    uint64_t totalPixels = (uint64_t)w * (uint64_t)h;

    bool tooManyPixels = (totalPixels > maxPixels);
    bool tooWide       = (w > deviceMaxDimension);
    bool tooTall       = (h > deviceMaxDimension);

    std::cout << "[Veloxr]" << "MaxPixels=" << maxPixels
              << " totalPixels=" << totalPixels
              << " deviceMaxDimension=" << deviceMaxDimension << "\n";
    std::cout << "[Veloxr]" << "Resolution (raw) = " << w << " x " << h
              << " Channels=" << texture.getNumChannels() << "\n";
    std::cout << "[Veloxr]" << "tooManyPixels=" << tooManyPixels
              << " tooWide=" << tooWide
              << " tooTall=" << tooTall << "\n";

    if (!tooManyPixels && !tooWide && !tooTall) {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4; // forcing RGBA
        one.pixelData = texture.load(texture.getFilename());
        result.tiles[0] = one;

        std::cout << "[Veloxr]" << "Loaded pixelData.size()=" << one.pixelData.size() << "\n";

        int orientation = texture.getOrientation();
        uint32_t orientedW = w;
        uint32_t orientedH = h;
        if (orientation == 6 || orientation == 8) {
            std::swap(orientedW, orientedH);
        }

        float left   = 0.0f;
        float right  = float(orientedW);
        float top    = 0.0f;
        float bottom = float(orientedH);
        int idx      = 0;
        one.samplerIndex = idx;

        std::vector<Vertex> singleTileVerts = {
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { left,  bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, top,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx },
        };

        for (auto &v : singleTileVerts) {
            glm::vec2 uv(v.texCoord.x, v.texCoord.y);

            glm::vec2 oldPos(v.pos.x, v.pos.y);
            glm::vec2 res;

            switch (orientation) {
                case 1:
                    std::cout << "[Veloxr]" << "Tile has no orientation change.\n";
                    res = uv; 
                    break;
                case 3:
                    std::cout << "[Veloxr]" << "Tile has 180 rotation.\n";
                    res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                    break;
                case 6:
                    std::cout << "[Veloxr]" << "Tile has 90 rotation.\n";
                    res = glm::vec2(uv.y, 1.0f - uv.x);
                    break;
                case 8:
                    std::cout << "[Veloxr]" << "Tile has 270 rotation.\n";
                    res = glm::vec2(1.0f - uv.y, uv.x);
                    break;
                default:
                    res = uv;
                    break;
            }
            v.texCoord.x = res.x;
            v.texCoord.y = res.y;
            //v.pos.x = newPos.x;
            //v.pos.y = newPos.y;
        }

        result.vertices.insert(result.vertices.end(),
                               singleTileVerts.begin(),
                               singleTileVerts.end());

        std::cout << "[Veloxr]" << "Single-tile approach used. \n";
        std::cout << "[Veloxr]" << "Tile " << idx
                  << " (" << one.width << " x " << one.height << ") completed\n";
        result.boundingBox = {0, 0, right, top};
        return result;
    }

    std::cout << "[Veloxr]" << "Texture too big for single tile. Doing multi-tiling.\n";

    uint32_t rawW = w; 
    uint32_t rawH = h;

    int orientation = texture.getOrientation();
    std::cout << "[Veloxr]" << "[INFO] Orientation = " << orientation << "\n";

    uint32_t orientedW = rawW;
    uint32_t orientedH = rawH;
    if (orientation == 6 || orientation == 8) {
        std::swap(orientedW, orientedH);
    }

    uint32_t Nx = (rawW + deviceMaxDimension - 1) / deviceMaxDimension;
    uint32_t Ny = (rawH + deviceMaxDimension - 1) / deviceMaxDimension;

    uint32_t tileW = (rawW + Nx - 1) / Nx;
    uint32_t tileH = (rawH + Ny - 1) / Ny;

    int totalTiles = Nx * Ny;
    int numThreads = std::min(totalTiles, 16);
    int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;

    // OIIO::ImageCache *ic = OIIO::ImageCache::create(true);
    std::shared_ptr<OIIO::ImageCache> ic = OIIO::ImageCache::create(true);
    ic->attribute("max_memory_MB", 1024.0f);
    OIIO::ImageSpec const *mainSpec = ic->imagespec(OIIO::ustring(texture.getFilename()), 0, 0);
    if (!mainSpec) {
        std::cerr << "[TILER] Could not get ImageSpec from ImageCache for file: "
                  << texture.getFilename() << "\n";
        OIIO::ImageCache::destroy(ic);
        return result;
    }

    uint32_t originalChannels = mainSpec->nchannels;
    uint32_t forcedChannels   = 4;

    struct ThreadResult {
        std::map<int, TextureData>         localTiles;
        std::map<int, std::vector<Vertex>> localVerts;
    };
    std::vector<ThreadResult> partialResults(numThreads);

    std::string filename = texture.getFilename();
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int t = 0; t < numThreads; t++) {
        int startIdx = t * tilesPerThread;
        int endIdx   = std::min(totalTiles, startIdx + tilesPerThread);

        threads.emplace_back([=, &partialResults, &ic]() {
            auto &localTiles = partialResults[t].localTiles;
            auto &localVerts = partialResults[t].localVerts;

            std::vector<unsigned char> readBuffer(size_t(tileW) * size_t(tileH) * size_t(originalChannels), 0);

            for (int idx = startIdx; idx < endIdx; idx++) {
                int row = idx / Nx;
                int col = idx % Nx;

                uint32_t x0 = col * tileW;
                uint32_t x1 = std::min(x0 + tileW, rawW);
                uint32_t y0 = row * tileH;
                uint32_t y1 = std::min(y0 + tileH, rawH);

                uint32_t thisTileW = (x1 > x0) ? (x1 - x0) : 0;
                uint32_t thisTileH = (y1 > y0) ? (y1 - y0) : 0;
                if (!thisTileW || !thisTileH) {
                    continue;
                }

                bool ok = ic->get_pixels(
                    OIIO::ustring(filename),   
                    0 /*subimage*/, 
                    0 /*miplevel*/,
                    (int)x0, (int)x1,
                    (int)y0, (int)y1,
                    0, 1,  // z range
                    0, (int)originalChannels,  // channel range
                    OIIO::TypeDesc::UINT8,
                    readBuffer.data()          // destination
                );
                if (!ok) {
                    std::cerr << "[TILER] Error reading tile " << idx
                              << " in thread " << t << ": "
                              << ic->geterror() << "\n";
                    continue;
                }

                std::vector<unsigned char> tileData(
                    size_t(thisTileW) * size_t(thisTileH) * size_t(forcedChannels),
                    255
                );

                for (uint32_t yy = 0; yy < thisTileH; yy++) {
                    size_t srcRowOffset = size_t(yy) * size_t(thisTileW) * size_t(originalChannels);
                    size_t dstRowOffset = size_t(yy) * size_t(thisTileW) * size_t(forcedChannels);
                    for (uint32_t xx = 0; xx < thisTileW; xx++) {
                        size_t srcPix = srcRowOffset + size_t(xx) * size_t(originalChannels);
                        size_t dstPix = dstRowOffset + size_t(xx) * size_t(forcedChannels);

                        for (uint32_t c = 0; c < forcedChannels; c++) {
                            if (c < originalChannels) {
                                tileData[dstPix + c] = readBuffer[srcPix + c];
                            } else {
                                tileData[dstPix + c] = 255;
                            }
                        }
                    }
                }

                TextureData data;
                data.width     = thisTileW;
                data.height    = thisTileH;
                data.channels  = forcedChannels;
                data.pixelData = std::move(tileData);
                data.samplerIndex = idx;
                localTiles[idx] = std::move(data);

                float tileLeft   = (float(x0));
                float tileRight  = (float(x1));
                float tileTop    = (float(y0));
                float tileBottom = (float(y1));

                Vertex v0 = { { tileLeft,  tileTop,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v1 = { { tileLeft,  tileBottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v2 = { { tileRight, tileBottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v3 = { { tileLeft,  tileTop,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v4 = { { tileRight, tileBottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v5 = { { tileRight, tileTop,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx };

                std::vector<Vertex> theseVerts = { v0, v1, v2, v3, v4, v5 };
                localVerts[idx] = std::move(theseVerts);

                std::cout << "[Veloxr]" << "Tile " << idx << " (thread " << t << ") completed.\n";
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    std::set<int> allIndices;
    for (int t = 0; t < numThreads; t++) {
        for (auto &kv : partialResults[t].localTiles) {
            allIndices.insert(kv.first);
        }
    }
    for (int idx : allIndices) {
        for (int t = 0; t < numThreads; t++) {
            auto itTiles = partialResults[t].localTiles.find(idx);
            if (itTiles != partialResults[t].localTiles.end()) {
                result.tiles[idx] = std::move(itTiles->second);
            }
            auto itVerts = partialResults[t].localVerts.find(idx);
            if (itVerts != partialResults[t].localVerts.end()) {
                auto &verts = itVerts->second;
                result.vertices.insert(result.vertices.end(),
                                       verts.begin(),
                                       verts.end());
            }
        }
    }

    applyExifOrientation(result.vertices, orientation, w, h );

    for(const auto& v : result.vertices) {
        result.boundingBox.x = std::min(v.pos.x, result.boundingBox.x);
        result.boundingBox.y = std::min(v.pos.y, result.boundingBox.y);
        result.boundingBox.z = std::max(v.pos.y, result.boundingBox.z);
        result.boundingBox.w = std::max(v.pos.y, result.boundingBox.w);
    }

    OIIO::ImageCache::destroy(ic);

    return result;
}


TiledResult TextureTiling::tile7(OIIOTexture &texture, uint32_t deviceMaxDimension)
{
    TiledResult result;
    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return result;
    }

    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;

    uint64_t maxPixels   = (uint64_t)deviceMaxDimension * (uint64_t)deviceMaxDimension;
    uint64_t totalPixels = (uint64_t)w * (uint64_t)h;

    bool tooManyPixels = (totalPixels > maxPixels);
    bool tooWide       = (w > deviceMaxDimension);
    bool tooTall       = (h > deviceMaxDimension);

    std::cout << "[Veloxr]" << "MaxPixels=" << maxPixels
              << " totalPixels=" << totalPixels
              << " deviceMaxDimension=" << deviceMaxDimension << "\n";
    std::cout << "[Veloxr]" << "Resolution (raw) = " << w << " x " << h
              << " Channels=" << texture.getNumChannels() << "\n";
    std::cout << "[Veloxr]" << "tooManyPixels=" << tooManyPixels
              << " tooWide=" << tooWide
              << " tooTall=" << tooTall << "\n";

    if (!tooManyPixels && !tooWide && !tooTall)
    {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4; 
        one.pixelData = texture.load(texture.getFilename());
        result.tiles[0] = one;

        std::cout << "[Veloxr]" << "Loaded pixelData.size()=" << one.pixelData.size() << "\n";

        int orientation = texture.getOrientation();
        uint32_t orientedW = w;
        uint32_t orientedH = h;
        if (orientation == 6 || orientation == 8) {
            std::swap(orientedW, orientedH);
        }

        float left   = 0.0f;
        float right  = float(orientedW);
        float top    = 0.0f;
        float bottom = float(orientedH);
        int idx      = 0;

        std::vector<Vertex> singleTileVerts = {
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { left,  bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, top,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx },
        };

        for (auto &v : singleTileVerts) {
            glm::vec2 uv(v.texCoord.x, v.texCoord.y);
            glm::vec2 res;
            switch (orientation) {
                case 1:
                    std::cout << "[Veloxr]" << "Tile has no orientation change.\n";
                    res = uv;
                    break;
                case 3:
                    std::cout << "[Veloxr]" << "Tile has 180° rotation.\n";
                    res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                    break;
                case 6:
                    std::cout << "[Veloxr]" << "Tile has 90° rotation.\n";
                    res = glm::vec2(uv.y, 1.0f - uv.x);
                    break;
                case 8:
                    std::cout << "[Veloxr]" << "Tile has 270° rotation.\n";
                    res = glm::vec2(1.0f - uv.y, uv.x);
                    break;
                default:
                    res = uv;
                    break;
            }
            v.texCoord.x = res.x;
            v.texCoord.y = res.y;
        }

        result.vertices.insert(result.vertices.end(),
                               singleTileVerts.begin(),
                               singleTileVerts.end());

        std::cout << "[Veloxr]" << "Single-tile approach used. \n";
        std::cout << "[Veloxr]" << "Tile " << idx
                  << " (" << one.width << " x " << one.height << ") completed\n";
        return result;
    } else {

        std::cout << "[Veloxr]" << "Texture too big for single tile. Doing multi-tiling.\n";

        uint32_t rawW = w; 
        uint32_t rawH = h;

        int orientation = texture.getOrientation();
        std::cout << "[Veloxr]" << "[INFO] Orientation = " << orientation << "\n";

        uint32_t orientedW = rawW;
        uint32_t orientedH = rawH;
        if (orientation == 6 || orientation == 8) {
            std::swap(orientedW, orientedH);
        }

        uint32_t Nx = (rawW + deviceMaxDimension - 1) / deviceMaxDimension;
        uint32_t Ny = (rawH + deviceMaxDimension - 1) / deviceMaxDimension;

        uint32_t tileW = (rawW + Nx - 1) / Nx;  
        uint32_t tileH = (rawH + Ny - 1) / Ny;

        int totalTiles = Nx * Ny;
        int numThreads = std::min(totalTiles, 16);
        int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;

        struct ThreadResult {
            std::map<int, TextureData>         localTiles;
            std::map<int, std::vector<Vertex>> localVerts;
        };
        std::vector<ThreadResult> partialResults(numThreads);

        auto filename = texture.getFilename();

        std::vector<std::thread> threads;
        threads.reserve(numThreads);

        for (int t = 0; t < numThreads; t++) {
            int startIdx = t * tilesPerThread;
            int endIdx   = std::min(totalTiles, startIdx + tilesPerThread);

            threads.emplace_back([=, &partialResults]() {
                    // Thread-local ImageInput, this shit aint thread safe
                    std::unique_ptr<OIIO::ImageInput> in = OIIO::ImageInput::open(filename);
                    if (!in) {
                    std::cerr << "Could not open file: " << filename << std::endl;
                    return;
                    }
                    const OIIO::ImageSpec &spec = in->spec();

                    bool fileIsTiled = (spec.tile_width > 0 && spec.tile_height > 0);
                    uint32_t originalChannels = spec.nchannels;
                    uint32_t forcedChannels   = 4;

                    auto &localTiles = partialResults[t].localTiles;
                    auto &localVerts = partialResults[t].localVerts;

                    for (int idx = startIdx; idx < endIdx; idx++) {
                    int row = idx / Nx;
                    int col = idx % Nx;

                    uint32_t x0 = col * tileW;
                    uint32_t x1 = std::min(x0 + tileW, rawW);
                    uint32_t y0 = row * tileH;
                    uint32_t y1 = std::min(y0 + tileH, rawH);

                    uint32_t thisTileW = (x1 > x0)? (x1 - x0) : 0;
                    uint32_t thisTileH = (y1 > y0)? (y1 - y0) : 0;
                    if (!thisTileW || !thisTileH) {
                        continue;
                    }

                    std::vector<unsigned char> tileData(
                            size_t(thisTileW)*size_t(thisTileH)*size_t(forcedChannels),
                            255
                            );

                    if (!fileIsTiled) {
                        std::vector<unsigned char> rowBuffer(size_t(rawW)*size_t(originalChannels));
                        std::vector<unsigned char> rgbaRow  (size_t(rawW)*size_t(forcedChannels), 255);

                        for (uint32_t scanY = y0; scanY < y1; scanY++) {
                            bool ok = in->read_scanline(scanY, 0, OIIO::TypeDesc::UINT8, rowBuffer.data());
                            if (!ok) {
                                std::cerr << "[TILER] Error reading scanline "
                                    << scanY << ": " << in->geterror() << std::endl;
                                break;
                            }
                            for (uint32_t x = 0; x < rawW; x++) {
                                for (uint32_t c = 0; c < originalChannels && c < forcedChannels; c++) {
                                    rgbaRow[size_t(x)*forcedChannels + c] =
                                        rowBuffer[size_t(x)*originalChannels + c];
                                }
                            }

                            size_t rowOffsetInTile = 
                                size_t(scanY - y0) * size_t(thisTileW) * size_t(forcedChannels);
                            size_t rowOffsetInBuf =
                                size_t(x0) * size_t(forcedChannels);

                            memcpy(&tileData[rowOffsetInTile],
                                    &rgbaRow[rowOffsetInBuf],
                                    size_t(thisTileW)*size_t(forcedChannels));
                        }
                    } else {
                        bool ok = in->read_tiles(
                                0, 0,       // mip level
                                int(x0), int(x1),
                                int(y0), int(y1),
                                0, 1,       // z range
                                0, 4,       // channels
                                OIIO::TypeDesc::UINT8,
                                tileData.data()
                                );
                        if (!ok) {
                            std::cerr << "[TILER] Error read_tiles " 
                                << y0 << ".." << y1 << ": "
                                << in->geterror() << std::endl;
                        }
                    }

                    TextureData data;
                    data.width     = thisTileW;
                    data.height    = thisTileH;
                    data.channels  = forcedChannels;
                    data.pixelData = std::move(tileData);

                    localTiles[idx] = std::move(data);

                    float tileLeft   = (float(x0) / float(rawW)) * float(orientedW);
                    float tileRight  = (float(x1) / float(rawW)) * float(orientedW);
                    float tileTop    = (float(y0) / float(rawH)) * float(orientedH);
                    float tileBottom = (float(y1) / float(rawH)) * float(orientedH);

                    Vertex v0 = {
                        { tileLeft,  tileTop,    0.0f, 0.0f },
                        { 0.0f,      0.0f, float(idx), 0.0f },
                        idx
                    };
                    Vertex v1 = {
                        { tileLeft,  tileBottom, 0.0f, 0.0f },
                        { 0.0f,      1.0f, float(idx), 0.0f },
                        idx
                    };
                    Vertex v2 = {
                        { tileRight, tileBottom, 0.0f, 0.0f },
                        { 1.0f,      1.0f, float(idx), 0.0f },
                        idx
                    };
                    Vertex v3 = {
                        { tileLeft,  tileTop,    0.0f, 0.0f },
                        { 0.0f,      0.0f, float(idx), 0.0f },
                        idx
                    };
                    Vertex v4 = {
                        { tileRight, tileBottom, 0.0f, 0.0f },
                        { 1.0f,      1.0f, float(idx), 0.0f },
                        idx
                    };
                    Vertex v5 = {
                        { tileRight, tileTop,    0.0f, 0.0f },
                        { 1.0f,      0.0f, float(idx), 0.0f },
                        idx
                    };

                    std::vector<Vertex> theseVerts = { v0, v1, v2, v3, v4, v5 };

                    // EXIF
                    for (auto &v : theseVerts) {
                        glm::vec2 uv(v.texCoord.x, v.texCoord.y);
                        glm::vec2 res;
                        switch (orientation) {
                            case 1:
                                res = uv;
                                break;
                            case 3:
                                res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                                break;
                            case 6:
                                res = glm::vec2(uv.y, 1.0f - uv.x);
                                break;
                            case 8:
                                res = glm::vec2(1.0f - uv.y, uv.x);
                                break;
                            default:
                                res = uv;
                                break;
                        }
                        v.texCoord.x = res.x;
                        v.texCoord.y = res.y;
                    }

                    localVerts[idx] = std::move(theseVerts);

                    std::cout << "[Veloxr]" << "Tile " << idx << " (thread " << t << ") completed.\n";
                    }
                    in->close();
            });
        }

        for (auto &th : threads) {
            th.join();
        }

        std::set<int> allIndices;
        for (int t = 0; t < numThreads; t++) {
            for (auto &kv : partialResults[t].localTiles) {
                allIndices.insert(kv.first);
            }
        }
        for (int idx : allIndices) {
            for (int t = 0; t < numThreads; t++) {
                auto itTiles = partialResults[t].localTiles.find(idx);
                if (itTiles != partialResults[t].localTiles.end()) {
                    result.tiles[idx] = std::move(itTiles->second);
                }
                auto itVerts = partialResults[t].localVerts.find(idx);
                if (itVerts != partialResults[t].localVerts.end()) {
                    auto &verts = itVerts->second;
                    result.vertices.insert(result.vertices.end(),
                            verts.begin(),
                            verts.end());
                }
            }
        }

        return result;

    }
}


TiledResult TextureTiling::tile6(OIIOTexture &texture, uint32_t deviceMaxDimension) {
    TiledResult result;
    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return result;
    }

    // Raw dimensions from the file ignoring EXIF
    uint32_t rawW = texture.getResolution().x;
    uint32_t rawH = texture.getResolution().y;

    uint64_t maxPixels   = (uint64_t)deviceMaxDimension * (uint64_t)deviceMaxDimension;
    uint64_t totalPixels = (uint64_t)rawW * (uint64_t)rawH;
    bool tooManyPixels   = (totalPixels > maxPixels);
    bool tooWide         = (rawW > deviceMaxDimension);
    bool tooTall         = (rawH > deviceMaxDimension);

    std::cout << "[Veloxr]" << "MaxPixels=" << maxPixels
              << " totalPixels=" << totalPixels
              << " deviceMaxDimension=" << deviceMaxDimension << "\n";
    std::cout << "[Veloxr]" << "Resolution (raw) = " << rawW << " x " << rawH
              << " Channels=" << texture.getNumChannels() << "\n";
    std::cout << "[Veloxr]" << "tooManyPixels=" << tooManyPixels
              << " tooWide=" << tooWide
              << " tooTall=" << tooTall << "\n";

    if (!tooManyPixels && !tooWide && !tooTall) { 

        int orientation = texture.getOrientation();


        uint32_t w = rawW;
        uint32_t h = rawH;
        TextureData one;
        one.width = w;
        one.height = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());
        result.tiles[0] = one;

        std::cout << "[Veloxr]" << "Loaded pixelData.size()=" << one.pixelData.size() << "\n";
        if (orientation == 6 || orientation == 8) {
            std::swap(w, h);
        }

        float left   = 0.0f;
        float right  = float(w);
        float top    = 0.0f;
        float bottom = float(h);
        int idx      = 0;

        std::vector<Vertex> singleTileVerts = {
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { left,  bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, top,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx },
        };

        for (auto &v : singleTileVerts) {
            glm::vec2 uv = { v.texCoord.x, v.texCoord.y };
            glm::vec2 res;

            switch (orientation) {
                case 1: {
                            std::cout << "[Veloxr]" << "Tile has no orientation change.\n";
                    res = uv;
                    break;
                }
                case 3: {
                            std::cout << "[Veloxr]" << "Tile has 180 rotation.\n";
                    res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                    break;
                }
                case 6: {
                            std::cout << "[Veloxr]" << "Tile has 90 rotation.\n";
                    res = glm::vec2(uv.y, 1.0f - uv.x);
                    break;
                }
                case 8: {
                            std::cout << "[Veloxr]" << "Tile has 270 rotation.\n";
                    res = glm::vec2(1.0f - uv.y, uv.x);
                    break; 
                }
                default: {
                    res = uv;
                    break;
                }
            }

            v.texCoord.x = res.x;
            v.texCoord.y = res.y;
        }

        result.vertices.insert(result.vertices.end(),
                              singleTileVerts.begin(),
                              singleTileVerts.end());

        std::cout << "[Veloxr]" << "Tile " << idx
                  << " (" << one.width << " x " << one.height << ") completed\n";
        return result;

    }

    int orientation = texture.getOrientation();  
    std::cout << "[Veloxr]" << "[INFO] Image EXIF orientation = " << orientation << "\n";

    uint32_t orientedW = rawW;
    uint32_t orientedH = rawH;
    if (orientation == 6 || orientation == 8) {
        std::swap(orientedW, orientedH);
        std::cout << "[Veloxr]" << "[DEBUG] Swapped orientedW/orientedH => "
            << orientedW << " x " << orientedH << "\n";
    }

    uint32_t Nx = (rawW + deviceMaxDimension - 1) / deviceMaxDimension;
    uint32_t Ny = (rawH + deviceMaxDimension - 1) / deviceMaxDimension;

    uint32_t tileW = (rawW + Nx - 1) / Nx;  
    uint32_t tileH = (rawH + Ny - 1) / Ny;  

    float aspectRatio = (float)orientedW / (float)orientedH;
    float stepX       = 2.0f / float(Nx);  
    float stepY       = 2.0f / float(Ny);  

    int totalTiles = Nx * Ny;

    int numThreads   = std::min(totalTiles, 16);
    int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;

    struct ThreadResult {
        std::map<int, TextureData>        localTiles;
        std::map<int, std::vector<Vertex>> localVerts;
    };
    std::vector<ThreadResult> partialResults(numThreads);

    auto filename = texture.getFilename();

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int t = 0; t < numThreads; t++) {
        int startIdx = t * tilesPerThread;
        int endIdx   = std::min(totalTiles, startIdx + tilesPerThread);

        threads.emplace_back([=, &partialResults]() {
                std::unique_ptr<OIIO::ImageInput> in = OIIO::ImageInput::open(filename);
                if (!in) {
                std::cerr << "Could not open file: " << filename << std::endl;
                return;
                }
                const OIIO::ImageSpec &spec = in->spec();

                bool fileIsTiled = (spec.tile_width > 0 && spec.tile_height > 0);
                uint32_t originalChannels = spec.nchannels;
                uint32_t forcedChannels   = 4; 

                auto &localTiles = partialResults[t].localTiles;
                auto &localVerts = partialResults[t].localVerts;

                for (int idx = startIdx; idx < endIdx; idx++) {
                int row = idx / Nx;
                int col = idx % Nx;

                // subregion in RAW coordinates
                uint32_t x0 = col * tileW;
                uint32_t x1 = std::min(x0 + tileW, rawW);
                uint32_t y0 = row * tileH;
                uint32_t y1 = std::min(y0 + tileH, rawH);

                uint32_t thisTileW = (x1 > x0) ? (x1 - x0) : 0;
                uint32_t thisTileH = (y1 > y0) ? (y1 - y0) : 0;
                if (!thisTileW || !thisTileH) {
                    continue;
                }

                std::vector<unsigned char> tileData(
                        size_t(thisTileW) * size_t(thisTileH) * size_t(forcedChannels), 
                        255
                        );

                if (!fileIsTiled) {
                    std::vector<unsigned char> rowBuffer(size_t(rawW) * size_t(originalChannels));
                    std::vector<unsigned char> rgbaRow  (size_t(rawW) * size_t(forcedChannels), 255);

                    for (uint32_t scanY = y0; scanY < y1; scanY++) {
                        bool ok = in->read_scanline(
                                scanY, 0,
                                OIIO::TypeDesc::UINT8,
                                rowBuffer.data()
                                );
                        if (!ok) {
                            std::cerr << "[TILER] Error reading scanline "
                                << scanY << ": " << in->geterror() << std::endl;
                            break;
                        }

                        for (uint32_t x = 0; x < rawW; x++) {
                            for (uint32_t c = 0; c < originalChannels && c < forcedChannels; c++) {
                                rgbaRow[size_t(x) * size_t(forcedChannels) + c] =
                                    rowBuffer[size_t(x) * size_t(originalChannels) + c];
                            }
                        }

                        size_t rowOffsetInTile = 
                            size_t(scanY - y0) * size_t(thisTileW) * size_t(forcedChannels);
                        size_t rowOffsetInBuf  = 
                            size_t(x0) * size_t(forcedChannels);

                        memcpy(&tileData[rowOffsetInTile], 
                                &rgbaRow[rowOffsetInBuf], 
                                size_t(thisTileW) * size_t(forcedChannels));
                    }
                } else {
                    bool ok = in->read_tiles(
                            0, 0, // mip level
                            int(x0), int(x1),
                            int(y0), int(y1),
                            0, 1, // z range
                            0, 4,
                            OIIO::TypeDesc::UINT8,
                            tileData.data()
                            );
                    if (!ok) {
                        std::cerr << "[TILER] Error read_tiles " 
                            << y0 << ".." << y1 << ": "
                            << in->geterror() << std::endl;
                    }
                }

                TextureData data;
                data.width     = thisTileW;  
                data.height    = thisTileH;  
                data.channels  = forcedChannels;
                data.pixelData = std::move(tileData);

                localTiles[idx] = std::move(data);

                int rowFlipped = (Ny - 1 - row);

                float tileLeft   = -1.0f + float(col) * stepX * aspectRatio;
                float tileRight  = tileLeft + stepX * aspectRatio;
                float tileTop    =  1.0f - float(rowFlipped) * stepY;
                float tileBottom = tileTop - stepY;

                Vertex v0 = {
                    { tileLeft,  tileTop,    0.0f, 0.0f },
                    { 0.0f,      1.0f, float(idx), 0.0f },
                    idx
                };
                Vertex v1 = {
                    { tileRight, tileTop,    0.0f, 0.0f },
                    { 1.0f,      1.0f, float(idx), 0.0f },
                    idx
                };
                Vertex v2 = {
                    { tileRight, tileBottom, 0.0f, 0.0f },
                    { 1.0f,      0.0f, float(idx), 0.0f },
                    idx
                };
                Vertex v3 = {
                    { tileLeft,  tileTop,    0.0f, 0.0f },
                    { 0.0f,      1.0f, float(idx), 0.0f },
                    idx
                };
                Vertex v4 = {
                    { tileRight, tileBottom, 0.0f, 0.0f },
                    { 1.0f,      0.0f, float(idx), 0.0f },
                    idx
                };
                Vertex v5 = {
                    { tileLeft,  tileBottom, 0.0f, 0.0f },
                    { 0.0f,      0.0f, float(idx), 0.0f },
                    idx
                };

                std::vector<Vertex> verts = { v0, v1, v2, v3, v4, v5 };

                for (auto &v : verts) {
                    glm::vec2 uv(v.texCoord.x, v.texCoord.y);
                    glm::vec2 res;
                    switch (orientation) {
                        // I am actually not sure if this is 180,90,270 but i think..?
                        case 1: // no rotation
                            res = uv;
                            break;
                        case 3: // 180
                            res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                            break;
                        case 6: // 90
                            res = glm::vec2(uv.y, 1.0f - uv.x);
                            break;
                        case 8: // 270
                            res = glm::vec2(1.0f - uv.y, uv.x);
                            break;
                        default:
                            res = uv;
                            break;
                    }
                    v.texCoord.x = res.x;
                    v.texCoord.y = res.y;
                }

                localVerts[idx] = std::move(verts);

                std::cout << "[Veloxr]" << "Tile " << idx << " has completed.\n";
                }

                in->close();
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    std::set<int> allIndices;
    for (int t = 0; t < numThreads; t++) {
        for (auto &kv : partialResults[t].localTiles) {
            allIndices.insert(kv.first);
        }
    }

    for (int idx : allIndices) {
        for (int t = 0; t < numThreads; t++) {
            auto itTiles = partialResults[t].localTiles.find(idx);
            if (itTiles != partialResults[t].localTiles.end()) {
                result.tiles[idx] = std::move(itTiles->second);
            }
            auto itVerts = partialResults[t].localVerts.find(idx);
            if (itVerts != partialResults[t].localVerts.end()) {
                auto &theseVerts = itVerts->second;
                result.vertices.insert(result.vertices.end(),
                        theseVerts.begin(),
                        theseVerts.end());
            }
        }
    }

    return result;
}


// deviceMaxDimension is typically 16384, whatever is one side max
TiledResult TextureTiling::tile4(OIIOTexture &texture, uint32_t deviceMaxDimension) {

    TiledResult result;
    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return result;
    }

    uint64_t maxPixels = (uint64_t)deviceMaxDimension * (uint64_t)deviceMaxDimension;
    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;
    uint64_t totalPixels = (uint64_t)w * (uint64_t)h;
    bool tooManyPixels = (totalPixels > maxPixels);
    bool tooWide = (w > deviceMaxDimension);
    bool tooTall = (h > deviceMaxDimension);

    std::cout << "[Veloxr]" << "MaxPixels=" << maxPixels
              << " totalPixels=" << totalPixels
              << " deviceMaxDimension=" << deviceMaxDimension << "\n";
    std::cout << "[Veloxr]" << "Resolution=" << w << "x" << h
              << " Channels=" << texture.getNumChannels() << "\n";
    std::cout << "[Veloxr]" << "tooManyPixels=" << tooManyPixels
              << " tooWide=" << tooWide
              << " tooTall=" << tooTall << "\n";

    if (true || (!tooManyPixels && !tooWide && !tooTall)) {

        int orientation = texture.getOrientation();


        TextureData one;
        one.width = w;
        one.height = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());
        result.tiles[0] = one;

        std::cout << "[Veloxr]" << "Loaded pixelData.size()=" << one.pixelData.size() << "\n";
        if (orientation == 6 || orientation == 8) {
            std::swap(w, h);
        }

        float left   = 0.0f;
        float right  = float(w);
        float top    = 0.0f;
        float bottom = float(h);
        int idx      = 0;

        std::vector<Vertex> singleTileVerts = {
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { left,  bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { left,  top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, top,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx },
        };

        for (auto &v : singleTileVerts) {
            glm::vec2 uv = { v.texCoord.x, v.texCoord.y };
            glm::vec2 res;

            switch (orientation) {
                case 1: {
                            std::cout << "[Veloxr]" << "Tile has no orientation change.\n";
                    res = uv;
                    break;
                }
                case 3: {
                            std::cout << "[Veloxr]" << "Tile has 180 rotation.\n";
                    res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                    break;
                }
                case 6: {
                            std::cout << "[Veloxr]" << "Tile has 90 rotation.\n";
                    res = glm::vec2(uv.y, 1.0f - uv.x);
                    break;
                }
                case 8: {
                            std::cout << "[Veloxr]" << "Tile has 270 rotation.\n";
                    res = glm::vec2(1.0f - uv.y, uv.x);
                    break; 
                }
                default: {
                    res = uv;
                    break;
                }
            }

            v.texCoord.x = res.x;
            v.texCoord.y = res.y;
        }

        result.vertices.insert(result.vertices.end(),
                              singleTileVerts.begin(),
                              singleTileVerts.end());

        std::cout << "[Veloxr]" << "Tile " << idx
                  << " (" << one.width << " x " << one.height << ") completed\n";
        return result;
    }

    return result;
}



TiledResult TextureTiling::tile5(OIIOTexture &texture, uint32_t deviceMaxDimension) {
    TiledResult result;
    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return result;
    }

    uint64_t maxPixels = (uint64_t)deviceMaxDimension * (uint64_t)deviceMaxDimension;
    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;
    float aspectRatio = (float)w / (float)h;
    uint64_t totalPixels = (uint64_t)w * (uint64_t)h;
    bool tooManyPixels = (totalPixels > maxPixels);
    bool tooWide       = (w > deviceMaxDimension);
    bool tooTall       = (h > deviceMaxDimension);

    if (!tooManyPixels && !tooWide && !tooTall) {
        TextureData one;
        one.width = w;
        one.height = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());
        result.tiles[0] = one;

        float left   = -1.0f * aspectRatio;
        float right  = +1.0f * aspectRatio;
        float top    = +1.0f;
        float bottom = -1.0f;
        int idx      = 0;

        std::vector<Vertex> singleTileVerts = {
            { { left,   top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { left,   bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right,  bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },

            { { left,   top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { right,  bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right,  top,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx },
        };
        result.vertices.insert(result.vertices.end(), singleTileVerts.begin(), singleTileVerts.end());
        std::cout << "[Veloxr]" << "Tile " << idx << " has completed.\n";
        return result;
    }

    uint32_t Nx = (w + deviceMaxDimension - 1) / deviceMaxDimension;
    uint32_t Ny = (h + deviceMaxDimension - 1) / deviceMaxDimension;
    uint32_t tileW = (w + Nx - 1) / Nx;
    uint32_t tileH = (h + Ny - 1) / Ny;

    float stepX = 2.0f / float(Nx);
    float stepY = 2.0f / float(Ny);
    int totalTiles = Nx * Ny;

    struct ThreadResult {
        std::map<int, TextureData> localTiles;
        std::map<int, std::vector<Vertex>> localVerts;
    };
    int numThreads = std::min(totalTiles, 16);
    std::vector<ThreadResult> partialResults(numThreads);
    int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;
    auto filename = texture.getFilename();

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int t = 0; t < numThreads; t++) {
        int startIdx = t * tilesPerThread;
        int endIdx   = std::min(totalTiles, startIdx + tilesPerThread);

        threads.push_back(std::thread([=, &partialResults]() {
            std::unique_ptr<OIIO::ImageInput> in = OIIO::ImageInput::open(filename);
            if (!in) {
                std::cerr << "Could not open file: " << filename << std::endl;
                return;
            }
            const OIIO::ImageSpec &spec = in->spec();

            int orientation = spec.get_int_attribute("Orientation", 1);
            if (orientation != 1) {
                std::cerr << "[DEBUG] Large image orientation = " << orientation 
                          << " (not 1). Might need rotate/flip.\n";
            }

            bool fileIsTiled = (spec.tile_width > 0 && spec.tile_height > 0);
            uint32_t originalChannels = spec.nchannels;
            uint32_t forcedChannels   = 4;

            auto &localTiles = partialResults[t].localTiles;
            auto &localVerts = partialResults[t].localVerts;

            for (int idx = startIdx; idx < endIdx; idx++) {
                int row = idx / Nx;
                int col = idx % Nx;
                uint32_t x0 = col * tileW;
                uint32_t x1 = std::min(x0 + tileW, w);
                uint32_t y0 = row * tileH;
                uint32_t y1 = std::min(y0 + tileH, h);
                uint32_t thisTileW = (x1 > x0) ? (x1 - x0) : 0;
                uint32_t thisTileH = (y1 > y0) ? (y1 - y0) : 0;
                if (!thisTileW || !thisTileH) {
                    continue;
                }

                std::vector<unsigned char> tileData(
                    size_t(thisTileW) * size_t(thisTileH) * size_t(forcedChannels), 
                    255
                );

                if (!fileIsTiled) {
                    std::vector<unsigned char> rowBuffer(size_t(w) * size_t(originalChannels));
                    std::vector<unsigned char> rgbaRow(size_t(w) * size_t(forcedChannels), 255);

                    for (uint32_t scanY = y0; scanY < y1; scanY++) {
                        bool ok = in->read_scanline(scanY, 0, OIIO::TypeDesc::UINT8, rowBuffer.data());

                        if (!ok) {
                            std::cerr << "[TILER] Error reading scanline " << scanY
                                      << ": " << in->geterror() << std::endl;
                            break;
                        }
                        for (uint32_t x = 0; x < w; x++) {
                            for (uint32_t c = 0; c < originalChannels && c < forcedChannels; c++) {
                                rgbaRow[size_t(x) * size_t(forcedChannels) + c] =
                                    rowBuffer[size_t(x) * size_t(originalChannels) + c];
                            }
                        }
                        size_t rowOffsetInTile = 
                            size_t(scanY - y0) * size_t(thisTileW) * size_t(forcedChannels);
                        size_t rowOffsetInBuf  = 
                            size_t(x0) * size_t(forcedChannels);

                        memcpy(&tileData[rowOffsetInTile], 
                               &rgbaRow[rowOffsetInBuf], 
                               size_t(thisTileW) * size_t(forcedChannels));
                    }
                } else {
                    bool ok = in->read_tiles(0, 0,
                        int(x0), int(x1),
                        int(y0), int(y1),
                        0, 1, // z range
                        0, 4,
                        OIIO::TypeDesc::UINT8,
                        tileData.data()
                    );
                    if (!ok) {
                        std::cerr << "[TILER] Error read_tiles " << y0 << ".." 
                                  << y1 << ": " << in->geterror() << std::endl;
                    }
                }

                TextureData data;
                data.width    = thisTileW;
                data.height   = thisTileH;
                data.channels = forcedChannels;
                data.pixelData = std::move(tileData);
                localTiles[idx] = data;

                int rowFlipped = (Ny - 1 - row);
                float tileLeft   = -1.0f + float(col) * stepX * aspectRatio;
                float tileRight  = tileLeft + stepX * aspectRatio;
                float tileTop    =  1.0f - float(rowFlipped) * stepY;
                float tileBottom = tileTop - stepY;

                Vertex v0 = { { tileLeft,  tileTop,    0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v1 = { { tileRight, tileTop,    0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v2 = { { tileRight, tileBottom, 0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v3 = { { tileLeft,  tileTop,    0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v4 = { { tileRight, tileBottom, 0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v5 = { { tileLeft,  tileBottom, 0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx };

                localVerts[idx] = { v0, v1, v2, v3, v4, v5 };

                std::cout << "[Veloxr]" << "Tile " << idx << " has completed.\n";
            }
            in->close();
        }));
    }

    for (auto &th : threads) {
        th.join();
    }

    std::set<int> allIndices;
    for (int t = 0; t < numThreads; t++) {
        for (auto &kv : partialResults[t].localTiles) {
            allIndices.insert(kv.first);
        }
    }

    for (int idx : allIndices) {
        for (int t = 0; t < numThreads; t++) {
            auto itTiles = partialResults[t].localTiles.find(idx);
            if (itTiles != partialResults[t].localTiles.end()) {
                result.tiles[idx] = itTiles->second;
            }
            auto itVerts = partialResults[t].localVerts.find(idx);
            if (itVerts != partialResults[t].localVerts.end()) {
                result.vertices.insert(result.vertices.end(),
                                       itVerts->second.begin(),
                                       itVerts->second.end());
            }
        }
    }
    return result;
}



