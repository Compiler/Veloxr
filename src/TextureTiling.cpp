#include "TextureTiling.h"
#include <OpenImageIO/imageio.h>
#include <cmath>
#include <iostream>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

TiledResult TextureTiling::tile5(OIIOTexture &texture, uint32_t maxResolution) {
    if (!texture.isInitialized()) {
        return {};
    }

    TiledResult result;
    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;
    uint64_t totalPixels = (uint64_t)w * (uint64_t)h;
    float aspect = (float)w / (float)h;

    if (totalPixels <= maxResolution) {
        TextureData one;
        one.width = w;
        one.height = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());
        result.tiles.push_back(one);

        float LEFT = -aspect;
        float RIGHT = aspect;
        float TOP = -1.0f;
        float BOTTOM = 1.0f;

        uint32_t i = 0;

        Vertex v0 = { {LEFT,  TOP,    0.f, 0.f}, {0.f, 0.f, (float)i, 0.f}, (int)i };
        Vertex v1 = { {RIGHT, BOTTOM, 0.f, 0.f}, {1.f, 1.f, (float)i, 0.f}, (int)i };
        Vertex v2 = { {RIGHT, TOP,    0.f, 0.f}, {1.f, 0.f, (float)i, 0.f}, (int)i };
        Vertex v3 = { {LEFT,  TOP,    0.f, 0.f}, {0.f, 0.f, (float)i, 0.f}, (int)i };
        Vertex v4 = { {LEFT,  BOTTOM, 0.f, 0.f}, {0.f, 1.f, (float)i, 0.f}, (int)i };
        Vertex v5 = { {RIGHT, BOTTOM, 0.f, 0.f}, {1.f, 1.f, (float)i, 0.f}, (int)i };

        result.vertices.insert(result.vertices.end(), {v0, v1, v2, v3, v4, v5});
        return result;
    }

    double ratio = (double)totalPixels / (double)maxResolution;
    double exactN = std::sqrt(ratio);
    int N = (int)std::ceil(exactN);
    uint32_t tileW = (w + N - 1) / N;
    uint32_t tileH = (h + N - 1) / N;

    float totalWidth = 2.0f * aspect;
    float totalHeight = 2.0f;
    float stepX = totalWidth / float(N);
    float stepY = totalHeight / float(N);

    int totalTiles = N * N;
    std::vector<TextureData> tileResults(totalTiles);
    std::vector<std::vector<Vertex>> vertexResults(totalTiles);

    int numThreads = 16;
    int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    auto filename = texture.getFilename();

    for (int t = 0; t < numThreads; t++) {
        int start = t * tilesPerThread;
        int end = std::min(totalTiles, start + tilesPerThread);

        threads.emplace_back([=, &tileResults, &vertexResults, &w, &h,
                              &tileW, &tileH, &stepX, &stepY]() {
            std::unique_ptr<OIIO::ImageInput> in = OIIO::ImageInput::open(filename);
            if (!in) {
                return;
            }
            const OIIO::ImageSpec &spec = in->spec();
            uint32_t originalChannels = spec.nchannels;
            uint32_t forcedChannels = 4;

            for (int idx = start; idx < end; idx++) {
                int row = idx / N;
                int col = idx % N;
                uint32_t x0 = col * tileW;
                uint32_t x1 = std::min(x0 + tileW, w);
                uint32_t y0 = row * tileH;
                uint32_t y1 = std::min(y0 + tileH, h);
                uint32_t thisTileW = x1 - x0;
                uint32_t thisTileH = y1 - y0;
                if (thisTileW == 0 || thisTileH == 0) {
                    continue;
                }

                std::vector<unsigned char> tileData(thisTileW * thisTileH * forcedChannels, 255);
                std::vector<unsigned char> rowBuffer(w * originalChannels);
                std::vector<unsigned char> rgbaRow(w * forcedChannels, 255);

                for (int yy = y0; yy < (int)y1; yy++) {
                    bool ok = in->read_scanline(yy, 0, OIIO::TypeDesc::UINT8, rowBuffer.data());
                    if (!ok) {
                        break;
                    }
                    for (int x = 0; x < (int)w; x++) {
                        for (uint32_t c = 0; c < originalChannels && c < forcedChannels; c++) {
                            rgbaRow[x * forcedChannels + c] = rowBuffer[x * originalChannels + c];
                        }
                    }
                    size_t rowOffsetInTile = (yy - y0) * thisTileW * forcedChannels;
                    size_t rowOffsetInBuf = x0 * forcedChannels;
                    memcpy(&tileData[rowOffsetInTile], &rgbaRow[rowOffsetInBuf], thisTileW * forcedChannels);
                }

                TextureData data;
                data.width = thisTileW;
                data.height = thisTileH;
                data.channels = forcedChannels;
                data.pixelData = std::move(tileData);
                tileResults[idx] = std::move(data);

                float left = -aspect + col * stepX;
                float right = left + stepX;
                float top = -1.0f + row * stepY;
                float bottom = top + stepY;

                Vertex v0 = { {left,  top,    0.f, 0.f}, {0.f, 0.f, (float)idx, 0.f}, idx };
                Vertex v1 = { {right, bottom, 0.f, 0.f}, {1.f, 1.f, (float)idx, 0.f}, idx };
                Vertex v2 = { {right, top,    0.f, 0.f}, {1.f, 0.f, (float)idx, 0.f}, idx };
                Vertex v3 = { {left,  top,    0.f, 0.f}, {0.f, 0.f, (float)idx, 0.f}, idx };
                Vertex v4 = { {left,  bottom, 0.f, 0.f}, {0.f, 1.f, (float)idx, 0.f}, idx };
                Vertex v5 = { {right, bottom, 0.f, 0.f}, {1.f, 1.f, (float)idx, 0.f}, idx };

                vertexResults[idx] = {v0, v1, v2, v3, v4, v5};
            }
            in->close();
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    for (int i = 0; i < totalTiles; i++) {
        if (tileResults[i].width > 0 && tileResults[i].height > 0) {
            result.tiles.push_back(std::move(tileResults[i]));
            result.vertices.insert(result.vertices.end(),
                                   vertexResults[i].begin(),
                                   vertexResults[i].end());
        }
    }
    return result;
}


TiledResult TextureTiling::tile4(OIIOTexture &texture, uint32_t maxResolution){
    // n^2 * ~4k < 25*4k: Fit tiles into 100,000x100,000
    static std::vector<int> TILES = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121};

    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return {};
    }

    TiledResult result; 

    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;
    uint32_t totalPixels = w * h;

    if (totalPixels <= maxResolution) {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());

        result.tiles.push_back(one);

        uint32_t tileIndex = 0;

        float LEFT   = -1.0f, RIGHT = +1.0f;
        float TOP    = -1.0f, BOTTOM = 1.0f;

        std::vector<Vertex> singleTileVerts = {
            {{RIGHT, BOTTOM, 0, 0}, {1.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{LEFT,  BOTTOM, 0, 0}, {0.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{LEFT,  TOP,    0, 0}, {0.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},

            {{LEFT,  TOP,    0, 0}, {0.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{RIGHT, TOP,    0, 0}, {1.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{RIGHT, BOTTOM, 0, 0}, {1.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
        };

        result.vertices.insert(result.vertices.end(),
                singleTileVerts.begin(), singleTileVerts.end());

        return result;
    }

    double ratio = (double)totalPixels / (double)maxResolution;
    double exactN = std::sqrt(ratio);
    int N = (int)std::ceil(exactN);
    uint32_t tileW = (w + N - 1) / N;
    uint32_t tileH = (h + N - 1) / N;

    std::cout << "[Tiler] tileW, tileH: " << tileW << ", " << tileH << std::endl;
    std::cout << "[Tiler] Ratio: " << ratio << std::endl;
    std::cout << "[Tiler] Exact N: " << exactN << std::endl;
    std::cout << "[Tiler] N: " << N << std::endl;

    auto filename = texture.getFilename();

    float stepX = 2.0f / float(N);
    float stepY = 2.0f / float(N);

    int totalTiles = N * N;
    std::vector<TextureData> tileResults(totalTiles);
    std::vector<std::vector<Vertex>> vertexResults(totalTiles);

    int numThreads = 16;
    int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; t++) {
        int start = t * tilesPerThread;
        int end = std::min(totalTiles, start + tilesPerThread);
        threads.push_back(std::thread([=, &tileResults, &vertexResults, &w, &h, &tileW, &tileH, &stepX, &stepY]() {
            std::unique_ptr<OIIO::ImageInput> in = OIIO::ImageInput::open(filename);
            if (!in) {
                std::cerr << "Thread: Could not open file: " << filename << std::endl;
                return;
            }
            const OIIO::ImageSpec &spec = in->spec();
            uint32_t originalChannels = spec.nchannels;
            uint32_t forcedChannels   = 4;

            for (int idx = start; idx < end; idx++) {
                int row = idx / N;
                int col = idx % N;
                uint32_t x0 = col * tileW;
                uint32_t x1 = std::min(x0 + tileW, w);
                uint32_t y0 = row * tileH;
                uint32_t y1 = std::min(y0 + tileH, h);
                uint32_t thisTileW = x1 - x0;
                uint32_t thisTileH = y1 - y0;
                if (thisTileW <= 0 || thisTileH <= 0) {
                    continue;
                }
                std::vector<unsigned char> tileData(thisTileW * thisTileH * forcedChannels, 255);
                std::vector<unsigned char> rowBuffer(w * originalChannels);
                std::vector<unsigned char> rgbaRow(w * forcedChannels, 255);

                for (int yy = y0; yy < (int)y1; yy++) {
                    bool ok = in->read_scanline(yy, 0, OIIO::TypeDesc::UINT8, rowBuffer.data());
                    if (!ok) {
                        std::cerr << "Thread error reading scanline " << yy << ": " << in->geterror() << std::endl;
                        break;
                    }
                    for (int x = 0; x < (int)w; x++) {
                        for (uint32_t c = 0; c < originalChannels && c < forcedChannels; c++) {
                            rgbaRow[x * forcedChannels + c] = rowBuffer[x * originalChannels + c];
                        }
                    }
                    size_t rowOffsetInTile = (yy - y0) * thisTileW * forcedChannels;
                    size_t rowOffsetInBuf  = x0 * forcedChannels;
                    memcpy(&tileData[rowOffsetInTile],
                           &rgbaRow[rowOffsetInBuf],
                           thisTileW * forcedChannels);
                }
                TextureData data;
                data.width    = thisTileW;
                data.height   = thisTileH;
                data.channels = forcedChannels;
                data.pixelData = std::move(tileData);
                tileResults[idx] = std::move(data);

                float left   = -1.0f + float(col) * stepX;
                float right  = left + stepX;
                float top    = 1.0f - float(row) * stepY;
                float bottom = top - stepY;

                Vertex v0 = { { right, -bottom, 0.0f, 0.0f },
                              { 1.0f,   1.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v1 = { { left, -bottom, 0.0f, 0.0f },
                              { 0.0f,   1.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v2 = { { left, -top,    0.0f, 0.0f },
                              { 0.0f,   0.0f,   float(idx), 0.0f },
                              (int) idx };

                Vertex v3 = { { left, -top,    0.0f, 0.0f },
                              { 0.0f,   0.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v4 = { { right, -top,    0.0f, 0.0f },
                              { 1.0f,   0.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v5 = { { right, -bottom, 0.0f, 0.0f },
                              { 1.0f,   1.0f,   float(idx), 0.0f },
                              (int) idx };

                vertexResults[idx] = {v0, v1, v2, v3, v4, v5};

                std::cout << "[TILER] Done with tile " << idx << "\n";
            }
            in->close();
        }));
    }
    for (auto &th : threads) {
        th.join();
    }
    for (int i = 0; i < totalTiles; i++) {
        if (tileResults[i].width > 0 && tileResults[i].height > 0) {
            result.tiles.push_back(std::move(tileResults[i]));
            result.vertices.insert(result.vertices.end(), vertexResults[i].begin(), vertexResults[i].end());
        }
    }
    return result;
}


TiledResult TextureTiling::tile3(OIIOTexture &texture, uint32_t maxResolution){
    // n^2 * ~4k < 25*4k: Fit tiles into 100,000x100,000
    static std::vector<int> TILES = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121};

    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return {};
    }

    TiledResult result; 

    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;
    uint32_t totalPixels = w * h;

    if (totalPixels <= maxResolution) {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());

        result.tiles.push_back(one);

        uint32_t tileIndex = 0;

        float LEFT   = -1.0f, RIGHT = +1.0f;
        float TOP    = -1.0f, BOTTOM = 1.0f;

        std::vector<Vertex> singleTileVerts = {
            {{RIGHT, BOTTOM, 0, 0}, {1.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{LEFT,  BOTTOM, 0, 0}, {0.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{LEFT,  TOP,    0, 0}, {0.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},

            {{LEFT,  TOP,    0, 0}, {0.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{RIGHT, TOP,    0, 0}, {1.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{RIGHT, BOTTOM, 0, 0}, {1.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
        };

        result.vertices.insert(result.vertices.end(),
                singleTileVerts.begin(), singleTileVerts.end());

        return result;
    }

    double ratio = (double)totalPixels / (double)maxResolution;
    double exactN = std::sqrt(ratio);
    int N = (int)std::ceil(exactN);

    uint32_t tileW = (w + N - 1) / N;
    uint32_t tileH = (h + N - 1) / N;

    std::cout << "[Tiler] tileW, tileH: " << tileW << ", " << tileH << std::endl;
    std::cout << "[Tiler] Ratio: " << ratio << std::endl;
    std::cout << "[Tiler] Exact N: " << exactN << std::endl;
    std::cout << "[Tiler] N: " << N << std::endl;

    auto filename = texture.getFilename();
    std::unique_ptr<OIIO::ImageInput> in = OIIO::ImageInput::open(filename);
    if (!in) {
        std::cerr << "Could not open file: " << filename << std::endl;
    }
    const OIIO::ImageSpec &spec = in->spec();

    uint32_t originalChannels = spec.nchannels;
    uint32_t forcedChannels   = 4;

    float stepX = 2.0f / float(N);
    float stepY = 2.0f / float(N);

    std::vector<unsigned char> fullImage(w * h * originalChannels);
    for (uint32_t y = 0; y < h; y++) {
        bool ok = in->read_scanline(y, 0, OIIO::TypeDesc::UINT8, fullImage.data() + y * w * originalChannels);
        if (!ok) {
            std::cerr << "Error reading scanline: " << in->geterror() << std::endl;
            in->close();
            return result;
        }
    }
    in->close();

    int totalTiles = N * N;
    std::vector<TextureData> tileResults(totalTiles);
    std::vector<std::vector<Vertex>> vertexResults(totalTiles);

    int numThreads = 16;
    int tilesPerThread = (totalTiles + numThreads - 1) / numThreads;
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; t++) {
        int start = t * tilesPerThread;
        int end = std::min(totalTiles, start + tilesPerThread);
        threads.push_back(std::thread([=, &tileResults, &vertexResults, &fullImage]() {
            for (int idx = start; idx < end; idx++) {
                int row = idx / N;
                int col = idx % N;
                uint32_t x0 = col * tileW;
                uint32_t x1 = std::min(x0 + tileW, w);
                uint32_t y0 = row * tileH;
                uint32_t y1 = std::min(y0 + tileH, h);
                uint32_t thisTileW = x1 - x0;
                uint32_t thisTileH = y1 - y0;
                if (thisTileW <= 0 || thisTileH <= 0) {
                    continue;
                }
                std::vector<unsigned char> tileData(thisTileW * thisTileH * forcedChannels, 255);
                std::vector<unsigned char> rgbaRow(w * forcedChannels, 255);
                for (int yy = y0; yy < y1; yy++) {
                    const unsigned char* rowPtr = fullImage.data() + yy * w * originalChannels;
                    for (int x = 0; x < (int)w; x++) {
                        for (uint32_t c = 0; c < originalChannels && c < forcedChannels; c++) {
                            rgbaRow[x * forcedChannels + c] = rowPtr[x * originalChannels + c];
                        }
                    }
                    size_t rowOffsetInTile = (yy - y0) * thisTileW * forcedChannels;
                    size_t rowOffsetInBuf  = x0 * forcedChannels;
                    memcpy(&tileData[rowOffsetInTile],
                           &rgbaRow[rowOffsetInBuf],
                           thisTileW * forcedChannels);
                }

                TextureData data;
                data.width    = thisTileW;
                data.height   = thisTileH;
                data.channels = forcedChannels;
                data.pixelData = std::move(tileData);
                tileResults[idx] = std::move(data);

                float left   = -1.0f + float(col) * stepX;
                float right  = left + stepX;
                float top    = 1.0f - float(row) * stepY;
                float bottom = top - stepY;

                Vertex v0 = { { right, -bottom, 0.0f, 0.0f },
                              { 1.0f,   1.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v1 = { { left, -bottom, 0.0f, 0.0f },
                              { 0.0f,   1.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v2 = { { left, -top,    0.0f, 0.0f },
                              { 0.0f,   0.0f,   float(idx), 0.0f },
                              (int) idx };

                Vertex v3 = { { left, -top,    0.0f, 0.0f },
                              { 0.0f,   0.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v4 = { { right, -top,    0.0f, 0.0f },
                              { 1.0f,   0.0f,   float(idx), 0.0f },
                              (int) idx };
                Vertex v5 = { { right, -bottom, 0.0f, 0.0f },
                              { 1.0f,   1.0f,   float(idx), 0.0f },
                              (int) idx };

                vertexResults[idx] = {v0, v1, v2, v3, v4, v5};

                std::cout << "[TILER] Done with tile " << idx << "\n";
            }
        }));
    }
    for (auto &th : threads) {
        th.join();
    }

    for (int i = 0; i < totalTiles; i++) {
        if (tileResults[i].width > 0 && tileResults[i].height > 0) {
            result.tiles.push_back(std::move(tileResults[i]));
            result.vertices.insert(result.vertices.end(), vertexResults[i].begin(), vertexResults[i].end());
        }
    }

    return result;
}


TiledResult TextureTiling::tile2(OIIOTexture &texture, uint32_t maxResolution){
    // n^2 * ~4k < 25*4k: Fit tiles into 100,000x100,000
    static std::vector<int> TILES = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121};

    if (!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return {};
    }

    TiledResult result; 

    uint32_t w = texture.getResolution().x;
    uint32_t h = texture.getResolution().y;
    uint32_t totalPixels = w * h;

    if (totalPixels <= maxResolution) {
        TextureData one;
        one.width    = w;
        one.height   = h;
        one.channels = 4;
        one.pixelData = texture.load(texture.getFilename());

        result.tiles.push_back(one);

        uint32_t tileIndex = 0;

        float LEFT   = -1.0f, RIGHT = +1.0f;
        float TOP    = -1.0f, BOTTOM = 1.0f;

        std::vector<Vertex> singleTileVerts = {
            {{RIGHT, BOTTOM, 0, 0}, {1.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{LEFT,  BOTTOM, 0, 0}, {0.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{LEFT,  TOP,    0, 0}, {0.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},

            {{LEFT,  TOP,    0, 0}, {0.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{RIGHT, TOP,    0, 0}, {1.0f, 0.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
            {{RIGHT, BOTTOM, 0, 0}, {1.0f, 1.0f, (float)tileIndex, 0.0f}, (int)tileIndex},
        };

        result.vertices.insert(result.vertices.end(),
                singleTileVerts.begin(), singleTileVerts.end());

        return result;
    }

    double ratio = (double)totalPixels / (double)maxResolution;
    double exactN = std::sqrt(ratio);
    int N = (int)std::ceil(exactN);
    uint32_t tileW = (w + N - 1) / N;
    uint32_t tileH = (h + N - 1) / N;

    std::cout << "[Tiler] tileW, tileH: " << tileW << ", " << tileH << std::endl;
    std::cout << "[Tiler] Ratio: " << ratio << std::endl;
    std::cout << "[Tiler] Exact N: " << exactN << std::endl;
    std::cout << "[Tiler] N: " << N << std::endl;

    auto filename = texture.getFilename();
    std::unique_ptr<OIIO::ImageInput> in = OIIO::ImageInput::open(filename);
    if (!in) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return result; // Empty
    }
    const OIIO::ImageSpec &spec = in->spec();

    uint32_t originalChannels = spec.nchannels;
    uint32_t forcedChannels   = 4;

    float stepX = 2.0f / float(N);
    float stepY = 2.0f / float(N);

    uint32_t tileIndex = 0;

    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            uint32_t x0 = col * tileW;
            uint32_t x1 = std::min(x0 + tileW, w);
            uint32_t y0 = row * tileH;
            uint32_t y1 = std::min(y0 + tileH, h);
            uint32_t thisTileW = x1 - x0;
            uint32_t thisTileH = y1 - y0;
            if (thisTileW <= 0 || thisTileH <= 0) {
                continue;
            }

            std::vector<unsigned char> tileData(thisTileW * thisTileH * forcedChannels, 255);
            std::vector<unsigned char> rowBuffer(w * originalChannels);
            std::vector<unsigned char> rgbaRow(w * forcedChannels, 255);

            for (int yy = y0; yy < (int)y1; ++yy) {
                bool ok = in->read_scanline(yy, 0, OIIO::TypeDesc::UINT8, rowBuffer.data());
                if (!ok) {
                    std::cerr << "Error reading scanline: " << in->geterror() << std::endl;
                    in->close();
                    return result; 
                }
                for (int x = 0; x < (int)w; ++x) {
                    for (uint32_t c = 0; c < originalChannels && c < forcedChannels; ++c) {
                        rgbaRow[x * forcedChannels + c] = rowBuffer[x * originalChannels + c];
                    }
                }
                size_t rowOffsetInTile = (yy - y0) * thisTileW * forcedChannels;
                size_t rowOffsetInBuf  = x0 * forcedChannels;
                memcpy(&tileData[rowOffsetInTile],
                        &rgbaRow[rowOffsetInBuf],
                        thisTileW * forcedChannels);
            }

            TextureData data;
            data.width    = thisTileW;
            data.height   = thisTileH;
            data.channels = forcedChannels;
            data.pixelData = std::move(tileData);
            result.tiles.push_back(std::move(data));

            float left   = -1.0f + float(col) * stepX;
            float right  = left + stepX;
            float top    =  1.0f - float(row) * stepY;
            float bottom =  top - stepY;

            Vertex v0 = { { right,  -bottom, 0.0f, 0.0f },
                { 1.0f,   1.0f,   (float)tileIndex, 0.0f },
                (int) tileIndex };
            Vertex v1 = { { left,   -bottom, 0.0f, 0.0f },
                { 0.0f,   1.0f,   (float)tileIndex, 0.0f },
                (int) tileIndex };
            Vertex v2 = { { left,   -top,    0.0f, 0.0f },
                { 0.0f,   0.0f,   (float)tileIndex, 0.0f },
                (int) tileIndex };

            Vertex v3 = { { left,   -top,    0.0f, 0.0f },
                { 0.0f,   0.0f,   (float)tileIndex, 0.0f },
                (int) tileIndex };
            Vertex v4 = { { right,  -top,    0.0f, 0.0f },
                { 1.0f,   0.0f,   (float)tileIndex, 0.0f },
                (int) tileIndex };
            Vertex v5 = { { right,  -bottom, 0.0f, 0.0f },
                { 1.0f,   1.0f,   (float)tileIndex, 0.0f },
                (int) tileIndex };

            result.vertices.push_back(v0);
            result.vertices.push_back(v1);
            result.vertices.push_back(v2);
            result.vertices.push_back(v3);
            result.vertices.push_back(v4);
            result.vertices.push_back(v5);

            std::cout << "[TILER] Done with tile " << tileIndex << "\n";
            tileIndex++;
        }
    }

    in->close();
    return result;
}
