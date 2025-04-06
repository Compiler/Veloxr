#include "TextureTiling.h"
#include <OpenImageIO/imageio.h>
#include <cmath>
#include <iostream>
#include <set>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

// deviceMaxDimension is typically 16384, whatever is one side max
TiledResult TextureTiling::tile5(OIIOTexture &texture, uint32_t deviceMaxDimension) {
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
    bool tooWide       = (w > deviceMaxDimension);
    bool tooTall       = (h > deviceMaxDimension);

    if (!tooManyPixels && !tooWide && !tooTall) {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());
        result.tiles[0] = one;

        float left   = -1.0f;
        float right  = +1.0f;
        float top    = -1.0f;
        float bottom = +1.0f;
        int idx      = 0;

        std::vector<Vertex> singleTileVerts = {
            {{left,  top,    0.0f, 0.0f}, {0.0f, 0.0f, float(idx), 0.0f}, idx},
            {{left,  bottom, 0.0f, 0.0f}, {0.0f, 1.0f, float(idx), 0.0f}, idx},
            {{right, bottom, 0.0f, 0.0f}, {1.0f, 1.0f, float(idx), 0.0f}, idx},
            {{left,  top,    0.0f, 0.0f}, {0.0f, 0.0f, float(idx), 0.0f}, idx},
            {{right, bottom, 0.0f, 0.0f}, {1.0f, 1.0f, float(idx), 0.0f}, idx},
            {{right, top,    0.0f, 0.0f}, {1.0f, 0.0f, float(idx), 0.0f}, idx},
        };
        result.vertices.insert(result.vertices.end(), singleTileVerts.begin(), singleTileVerts.end());
        std::cout << "Tile " << idx << " has completed.\n";
        return result;
    }

    uint32_t Nx = (w + deviceMaxDimension - 1) / deviceMaxDimension;
    uint32_t Ny = (h + deviceMaxDimension - 1) / deviceMaxDimension;
    uint32_t tileW = (w + Nx - 1) / Nx;
    uint32_t tileH = (h + Ny - 1) / Ny;
    float stepX = 2.0f / float(Nx);
    float stepY = 2.0f / float(Ny);
    int totalTiles = Nx * Ny;

    // Store partial results in thread-local containers
    struct ThreadResult {
        std::map<int, TextureData> localTiles;
        std::map<int, std::vector<Vertex>> localVerts;
    };

    int numThreads = 8;
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

                std::vector<unsigned char> tileData(thisTileW * thisTileH * forcedChannels, 255);
                std::vector<unsigned char> rowBuffer(w * originalChannels);
                std::vector<unsigned char> rgbaRow(w * forcedChannels, 255);

                for (uint32_t scanY = y0; scanY < y1; scanY++) {
                    bool ok = in->read_scanline(scanY, 0, OIIO::TypeDesc::UINT8, rowBuffer.data());
                    if (!ok) {
                        std::cerr << "[TILER] Error reading scanline " << scanY << ": " << in->geterror() << std::endl;
                        break;
                    }
                    for (uint32_t x = 0; x < w; x++) {
                        for (uint32_t c = 0; c < originalChannels && c < forcedChannels; c++) {
                            rgbaRow[x * forcedChannels + c] = rowBuffer[x * originalChannels + c];
                        }
                    }
                    size_t rowOffsetInTile = (scanY - y0) * (thisTileW * forcedChannels);
                    size_t rowOffsetInBuf  = x0 * forcedChannels;
                    memcpy(&tileData[rowOffsetInTile], &rgbaRow[rowOffsetInBuf], thisTileW * forcedChannels);
                }

                TextureData data;
                data.width    = thisTileW;
                data.height   = thisTileH;
                data.channels = forcedChannels;
                data.pixelData = std::move(tileData);

                localTiles[idx] = data;

                float left   = -1.0f + col * stepX;
                float right  = left + stepX;
                float top    = -1.0f + row * stepY;
                float bottom = top + stepY;

                Vertex v0 = { { left,   top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v1 = { { left,   bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v2 = { { right,  bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v3 = { { left,   top,    0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx };
                Vertex v4 = { { right,  bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx };
                Vertex v5 = { { right,  top,    0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx };

                localVerts[idx] = { v0, v1, v2, v3, v4, v5 };

                std::cout << "Tile " << idx << " has completed.\n";
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
    // build final TiledResult in ascending index
    for (int idx : allIndices) {
        for (int t = 0; t < numThreads; t++) {
            auto itTiles = partialResults[t].localTiles.find(idx);
            if (itTiles != partialResults[t].localTiles.end()) {
                result.tiles[idx] = itTiles->second;
            }
            auto itVerts = partialResults[t].localVerts.find(idx);
            if (itVerts != partialResults[t].localVerts.end()) {
                // preserve ascending tile order in result.vertices
                result.vertices.insert(result.vertices.end(),
                                       itVerts->second.begin(),
                                       itVerts->second.end());
            }
        }
    }

    return result;
}


