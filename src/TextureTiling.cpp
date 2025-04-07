#include "TextureTiling.h"
#include <OpenImageIO/imageio.h>
#include <cmath>
#include <iostream>
#include <set>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

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

    std::cout << "MaxPixels=" << maxPixels << " totalPixels=" << totalPixels << " deviceMaxDimension=" << deviceMaxDimension << "\n";
    std::cout << "Resolution=" << w << "x" << h << " Channels=" << texture.getNumChannels() << "\n";
    std::cout << "tooManyPixels=" << tooManyPixels << " tooWide=" << tooWide << " tooTall=" << tooTall << "\n";

    if (true || (!tooManyPixels && !tooWide && !tooTall)) {
        TextureData one;
        one.width = w;
        one.height = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());
        result.tiles[0] = one;

        std::cout << "Loaded pixelData.size()=" << one.pixelData.size() << "\n";

        float left = 0.0f;
        float right = w;
        float top = 0.0f;
        float bottom = h;
        int idx = 0;

        std::vector<Vertex> singleTileVerts = {
            { { left, top, 0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { left, bottom, 0.0f, 0.0f }, { 0.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { left, top, 0.0f, 0.0f }, { 0.0f, 0.0f, float(idx), 0.0f }, idx },
            { { right, bottom, 0.0f, 0.0f }, { 1.0f, 1.0f, float(idx), 0.0f }, idx },
            { { right, top, 0.0f, 0.0f }, { 1.0f, 0.0f, float(idx), 0.0f }, idx },
        };

        for (auto &v : singleTileVerts) {
            glm::vec2 uv = { v.texCoord.x, v.texCoord.y };
            glm::vec2 res;
            switch (texture.getOrientation()) {
                case 1:
                    res = uv; break;
                case 3:
                    res = glm::vec2(1.0f - uv.x, 1.0f - uv.y); break;
                case 6:
                    res = glm::vec2(uv.y, 1.0f - uv.x); break;
                case 8:
                    res = glm::vec2(1.0f - uv.y, uv.x); break;
                default:
                    res = uv; break;
            }
            v.texCoord.x = res.x;
            v.texCoord.y = res.y;
        }
        result.vertices.insert(result.vertices.end(), singleTileVerts.begin(), singleTileVerts.end());
        std::cout << "Tile " << idx << "(" << one.width << " x " << one.height << ") completed\n";
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
                    // read_scanline approach
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
                    // read_tiles approach for subregion x0..x1, y0..y1
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

                std::cout << "Tile " << idx << " has completed.\n";
            }
            in->close();
        }));
    }

    for (auto &th : threads) {
        th.join();
    }

    // gather all tile indices
    std::set<int> allIndices;
    for (int t = 0; t < numThreads; t++) {
        for (auto &kv : partialResults[t].localTiles) {
            allIndices.insert(kv.first);
        }
    }

    // build final TiledResult
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



