#include "texture.h"
#include <OpenImageIO/imageio.h>
#include <cstdint>
#include <iostream>
#include <vector>

OIIO_NAMESPACE_USING

using namespace Veloxr ;

OIIOTexture::OIIOTexture(std::string filename) {
    init(filename);
}

void OIIOTexture::init(std::string filename) {
    std::cout << "[VELOXR][TEXTURE] Loading texture: " << filename << '\n';
    _filename = filename;
    auto in = ImageInput::open(_filename);
    if (!in) {
        std::cerr << "Could not open input: " << _filename << "\n";
    }
    const ImageSpec &in_spec = in->spec();
    _resolution = {(size_t)in_spec.width, (size_t)in_spec.height};
    _numChannels = in_spec.nchannels;
    OIIO::ImageSpec spec = in->spec();
    auto orientation = spec.get_int_attribute("Orientation", 1);
    _orientation = orientation;
    console.debug("Filename = ", _filename, " Produces ", _numChannels, " channels at ", _resolution.x, "x", _resolution.y, " with orientation ", _orientation);
    in->close();
    in.reset();
    _loaded = true;
}

std::vector<unsigned char> OIIOTexture::load(std::string filename) {
    if (filename.empty() && !_loaded) {
        std::cerr << "OIIOTexture not initialized properly\n";
        static std::vector<unsigned char> err;
        return err;
    } else if (filename.empty() && _loaded) filename = _filename;
    if (!_loaded) init(filename);


    console.logc1("Opening image... ", filename);
    auto in = OIIO::ImageInput::open(filename);
    if (!in) {
        throw std::runtime_error("Failed to open image with OIIO: " + filename);
    }

    console.logc1("Done loading ", filename);
    console.logc1("Allocating buffer..", filename);

    std::vector<unsigned char> rawData(_resolution.x * _resolution.y * _numChannels);

    console.logc1("Reading image: ", _numChannels, " channel ", rawData.size() / 1024 / 1024, " mb.");
    in->read_image(0, 0, 0, _numChannels, OIIO::TypeDesc::UINT8, rawData.data());
    console.logc1("Done reading image.");
    in->close();
    in.reset();

    // rgba expansion
    const uint64_t w = _resolution.x;
    const uint64_t h = _resolution.y;
    const uint64_t pixels = w * h;

    std::vector<unsigned char> pixelData(static_cast<size_t>(pixels * 4), 255);

       for (uint64_t i = 0; i < pixels; ++i) {
        const uint64_t src = i * _numChannels;
        const uint64_t dst = i * 4;

        switch (_numChannels) {
        case 1:   // Gray-> R = G = B = L,  A = 255
            pixelData[dst + 0] = rawData[src + 0];
            pixelData[dst + 1] = rawData[src + 0];
            pixelData[dst + 2] = rawData[src + 0];
            break;

        case 2:   // Gray+Alpha -> R = G = B = L,  A = A
            pixelData[dst + 0] = rawData[src + 0];
            pixelData[dst + 1] = rawData[src + 0];
            pixelData[dst + 2] = rawData[src + 0];
            pixelData[dst + 3] = rawData[src + 1];
            break;

        case 3:   // RGB -> copy, leave α = 255 (already pre‑filled)
            pixelData[dst + 0] = rawData[src + 0];
            pixelData[dst + 1] = rawData[src + 1];
            pixelData[dst + 2] = rawData[src + 2];
            break;

        default:  // 4 or more channels -> assume the first four are RGBA
            pixelData[dst + 0] = rawData[src + 0];
            pixelData[dst + 1] = rawData[src + 1];
            pixelData[dst + 2] = rawData[src + 2];
            pixelData[dst + 3] = rawData[src + 3];
            break;
        }
    }

    _numChannels = 4;
    return pixelData;
}


