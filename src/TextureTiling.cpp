#include "TextureTiling.h"
#include <OpenImageIO/imageio.h>
#include <cmath>
#include <iostream>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

std::vector<std::vector<unsigned char>> TextureTiling::tile(OIIOTexture& texture, uint32_t maxResolution){
    // n^2 * ~4k < 25*4k: Fit tiles into 100,000x100,000
    static std::vector<int> TILES = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121};

    // Base case
    if(!texture.isInitialized()) {
        std::cerr << "Cannot tile a texture that is not initialized\n";
        return {{}};
    }

    long long w = texture.getResolution().x;
    long long h = texture.getResolution().y;
    long long totalPixels = w * h;

    if (totalPixels <= maxResolution) {
        return {texture.load()};
    }

    double ratio = (double)totalPixels / (double)maxResolution; 
    double exactN = std::sqrt(ratio);
    int N = (int)std::ceil(exactN);

    long long tileW = (w + N - 1) / N;
    long long tileH = (h + N - 1) / N;

    auto resolution = texture.getResolution();
    auto pixelResolution = resolution.x * resolution.y;
    int factor = std::ceil((double)pixelResolution / (double)maxResolution);
    int factorIndex = factor - 1;
    int numTiles = TILES[factorIndex];
    int roiResolution = pixelResolution / numTiles;
    std::cout << "[Tiler] Resolution of image: " << resolution.x  << " x " << resolution.y << std::endl;
    std::cout << "[Tiler] Image pixels: " << pixelResolution << std::endl;
    std::cout << "[Tiler] Max Resolution allowed: " << maxResolution << std::endl;
    std::cout << "[Tiler] Factor: " << factor << std::endl;
    std::cout << "[Tiler] Num tiles chosen: " << numTiles << std::endl;
    std::cout << "[Tiler] Num tiles chosen for square: " << N << std::endl;
    std::cout << "[Tiler] Num tiles chosen: " << N << std::endl;
    std::cout << "[Tiler] tileW, tileH: " << tileW << ", " << tileH << std::endl;


    std::vector<std::vector<unsigned char>> tilePixels;

    auto filename = texture.getFilename();
    std::unique_ptr<ImageInput> in = ImageInput::open(filename);
    if (!in) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return {{}};
    }
    const ImageSpec& spec = in->spec();

    uint32_t originalChannels = spec.nchannels;
    uint32_t forcedChannels = 4;
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            long long x0 = col * tileW;
            long long x1 = std::min(x0 + tileW, w);
            long long y0 = row * tileH;
            long long y1 = std::min(y0 + tileH, h);
            long long thisTileW = x1 - x0;
            long long thisTileH = y1 - y0;
            if (thisTileW <= 0 || thisTileH <= 0) {
                continue;
            }
            std::vector<unsigned char> tileData(thisTileW * thisTileH * forcedChannels, 255);
            std::vector<unsigned char> rowBuffer(w * originalChannels);
            std::vector<unsigned char> rgbaRow(w * forcedChannels, 255);
            for (int yy = y0; yy < y1; ++yy) {
                bool ok = in->read_scanline(yy, 0, TypeDesc::UINT8, rowBuffer.data());
                if (!ok) {
                    std::cerr << "Error reading scanline: " << in->geterror() << std::endl;
                    in->close();
                    return {{}};
                }
                for (int x = 0; x < (int)w; ++x) {
                    for (uint32_t c = 0; c < originalChannels && c < forcedChannels; ++c) {
                        rgbaRow[x * forcedChannels + c] = rowBuffer[x * originalChannels + c];
                    }
                }
                size_t rowOffsetInTile = (yy - y0) * thisTileW * forcedChannels;
                size_t rowOffsetInBuf = x0 * forcedChannels;
                memcpy(&tileData[rowOffsetInTile], &rgbaRow[rowOffsetInBuf], thisTileW * forcedChannels);
            }
            tilePixels.push_back(std::move(tileData));
            return tilePixels;
        }
    }


    return tilePixels;
}
