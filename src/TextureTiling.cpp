#include "TextureTiling.h"
#include "Common.h"
#include <OpenImageIO/imageio.h>
#include <cmath>
#include <iostream>
#include <set>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

#include <OpenImageIO/imagecache.h>
#include <OpenImageIO/ustring.h>
#include <thread>

TiledResult TextureTiling::tile(Veloxr::VeloxrBuffer& buffer, uint32_t deviceMaxDimension) {
    TiledResult result;
    if (buffer.data.empty()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return result;
    }

    const v_int totalSizeBytes = buffer.data.size() * sizeof(unsigned char);
    const v_int expectedTotalSizeBytes = buffer.width * buffer.height * buffer.numChannels;

    console.debug("Total size of buffer: ", totalSizeBytes, " bytes.");
    console.debug("Total expected size:  ", expectedTotalSizeBytes, " bytes.");
    console.debug("Buffer orientation: ", buffer.orientation);
    

    auto w = buffer.width;
    auto h = buffer.height;

    auto maxPixels = (v_int)deviceMaxDimension * (v_int)deviceMaxDimension;
    auto totalPixels = (v_int)w * (v_int)h;

    bool tooManyPixels = (totalPixels > maxPixels);
    bool tooWide = (w > deviceMaxDimension);
    bool tooTall = (h > deviceMaxDimension);

    std::cout << "[Veloxr]" << "MaxPixels=" << maxPixels
              << " totalPixels=" << totalPixels
              << " deviceMaxDimension=" << deviceMaxDimension << "\n";
    std::cout << "[Veloxr]" << "Resolution (raw) = " << w << " x " << h
              << " Channels=" << buffer.numChannels << "\n";
    std::cout << "[Veloxr]" << "tooManyPixels=" << tooManyPixels
              << " tooWide=" << tooWide
              << " tooTall=" << tooTall << "\n";

    if (!tooManyPixels && !tooWide && !tooTall) {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4; // forcing RGBA
        one.pixelData = buffer.data;
        result.tiles[0] = one;

        std::cout << "[Veloxr]" << "Loaded pixelData.size()=" << one.pixelData.size() << "\n";

        auto orientation = buffer.orientation;
        auto orientedW = w;
        auto orientedH = h;
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
                case Veloxr::EXIFCases::CW_180:
                    std::cout << "[Veloxr]" << "Tile has 180 rotation.\n";
                    res = glm::vec2(1.0f - uv.x, 1.0f - uv.y);
                    break;
                case Veloxr::EXIFCases::CW_90:
                    std::cout << "[Veloxr]" << "Tile has 90 rotation.\n";
                    res = glm::vec2(uv.y, 1.0f - uv.x);
                    break;
                case Veloxr::EXIFCases::CW_270:
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

    v_int rawW = w; 
    v_int rawH = h;

    v_int orientation = buffer.orientation;
    std::cout << "[Veloxr]" << "[INFO] Orientation = " << orientation << "\n";

    v_int orientedW = rawW;
    v_int orientedH = rawH;
    if (orientation == 6 || orientation == 8) {
        std::swap(orientedW, orientedH);
    }

    v_int Nx = (rawW + deviceMaxDimension - 1) / deviceMaxDimension;
    v_int Ny = (rawH + deviceMaxDimension - 1) / deviceMaxDimension;

    v_int tileW = (rawW + Nx - 1) / Nx;
    v_int tileH = (rawH + Ny - 1) / Ny;

    v_int totalTiles = Nx * Ny;
    v_int numThreads = std::min(totalTiles, (v_int)16);
    v_int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;

    // OIIO::ImageCache *ic = OIIO::ImageCache::create(true);
    std::shared_ptr<OIIO::ImageCache> ic = OIIO::ImageCache::create(true);
    ic->attribute("max_memory_MB", 1024.0f);

    v_int originalChannels = buffer.numChannels;
    v_int forcedChannels   = 4;

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

        threads.emplace_back([=, &partialResults, &buffer]() {
            auto &localTiles = partialResults[t].localTiles;
            auto &localVerts = partialResults[t].localVerts;

            std::vector<unsigned char> readBuffer;

            for (int idx = startIdx; idx < endIdx; idx++) {
                int row = idx / Nx;
                int col = idx % Nx;

                v_int x0 = col * tileW;
                v_int x1 = std::min(x0 + tileW, rawW);
                v_int y0 = row * tileH;
                v_int y1 = std::min(y0 + tileH, rawH);

                v_int thisTileW = (x1 > x0) ? (x1 - x0) : 0;
                v_int thisTileH = (y1 > y0) ? (y1 - y0) : 0;
                if (!thisTileW || !thisTileH) {
                    continue;
                }

                //row copies
                for (v_int yy = 0; yy < thisTileH; ++yy) {
                    const v_int srcOff = (v_int(y0 + yy) * rawW + x0) * originalChannels;
                    const v_int dstOff = v_int(yy) * thisTileW * originalChannels;
                    readBuffer.resize(thisTileW * thisTileH * originalChannels);
                    std::memcpy(readBuffer.data() + dstOff,
                            buffer.data.data() + srcOff,
                            v_int(thisTileW) * originalChannels);
                }
                // probe 1
                {
                    bool anyData = std::any_of(readBuffer.begin(), readBuffer.end(),
                            [](unsigned char b){ return b != 255; });
                    console.debug("[Tile ", idx, "] rows-copied ok = ", anyData);
                }
                bool ok = true;
                if (!ok) {
                    std::cerr << "[TILER] Error reading tile " << idx
                              << " in thread " << t << ": "
                              << ic->geterror() << "\n";
                    continue;
                }

                std::vector<unsigned char> tileData(
                    v_int(thisTileW) * v_int(thisTileH) * v_int(forcedChannels),
                    255
                );

                for (v_int yy = 0; yy < thisTileH; yy++) {
                    v_int srcRowOffset = v_int(yy) * v_int(thisTileW) * v_int(originalChannels);
                    v_int dstRowOffset = v_int(yy) * v_int(thisTileW) * v_int(forcedChannels);
                    for (v_int xx = 0; xx < thisTileW; xx++) {
                        v_int srcPix = srcRowOffset + v_int(xx) * v_int(originalChannels);
                        v_int dstPix = dstRowOffset + v_int(xx) * v_int(forcedChannels);

                        for (v_int c = 0; c < forcedChannels; c++) {
                            if (c < originalChannels) {
                                tileData[dstPix + c] = readBuffer[srcPix + c];
                            } else {
                                tileData[dstPix + c] = 255;
                            }
                            // // probe 2
                            // {
                            //     bool anyPix = std::any_of(tileData.begin(), tileData.end(),
                            //             [](unsigned char b){ return b != 255; });
                            //     probe_2_message += "[Tile " + std::to_string(idx) + "] swizzle ok = " + std::to_string(anyPix) + '\n';
                            // }
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
    using F = std::numeric_limits<float>;
    result.boundingBox = {  F::max(),  F::max(), -F::max(), -F::max() };
    for(const auto& v : result.vertices) {
        result.boundingBox.x = std::min(v.pos.x, result.boundingBox.x);
        result.boundingBox.y = std::min(v.pos.y, result.boundingBox.y);
        result.boundingBox.z = std::max(v.pos.x, result.boundingBox.z);
        result.boundingBox.w = std::max(v.pos.y, result.boundingBox.w);
    }

    OIIO::ImageCache::destroy(ic);

    return result;
}


void TextureTiling::applyExifOrientation( std::vector<Vertex>& vertices, int orientation, uint32_t rawW, uint32_t rawH) {
    for (auto& v : vertices) {
        float oldX = v.pos.x;
        float oldY = v.pos.y;

        float newX = oldX;
        float newY = oldY;

        switch (orientation) {
            case Veloxr::EXIFCases::CW_0: break;
            case Veloxr::EXIFCases::CW_180:
                    newX = float(rawW)  - oldX;
                    newY = float(rawH)  - oldY;
                    break;

            case Veloxr::EXIFCases::CW_90:
                    newX = oldY;
                    newY = float(rawW) - oldX;
                    break;

            case Veloxr::EXIFCases::CW_270:
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
            case Veloxr::EXIFCases::CW_0: break;
            case Veloxr::EXIFCases::CW_180:
                uvRes = glm::vec2(1.0f - w, u);
                break;
            case Veloxr::EXIFCases::CW_90:
                uvRes = glm::vec2(w, 1.0f - u);
                break;
            case Veloxr::EXIFCases::CW_270:
                uvRes = glm::vec2(1.0f - u, 1.0f - w);
                break;
            default:
                break;
        }
    }
}

glm::vec2 TextureTiling::rotatePositionForOrientation(const glm::vec2 &p, int orientation, float width, float height) {
    switch (orientation) {
        case 1: 
        default:return p;
        case Veloxr::EXIFCases::CW_180: return {width - p.x, height - p.y};
        case Veloxr::EXIFCases::CW_90: return {p.y, width - p.x};
        case Veloxr::EXIFCases::CW_270: return {height - p.y, p.x};
    }
}
