#include "texture.h"
#include <OpenImageIO/imageio.h>
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
    _resolution = {(vsize)in_spec.width, (vsize)in_spec.height};
    _numChannels = in_spec.nchannels;
    OIIO::ImageSpec spec = in->spec();
    auto orientation = spec.get_int_attribute("Orientation", 1);
    _orientation = orientation;
    std::cout << "[Veloxr]" << "Filename=" << _filename << " Orientation=" << orientation << "\n";
    in->close();
    in.reset();
    _loaded = true;
}

std::vector<unsigned char> OIIOTexture::load(std::string filename, bool force4Channels ) {
    if (filename.empty() && !_loaded) {
        std::cerr << "OIIOTexture not initialized properly\n";
        static std::vector<unsigned char> err;
        return err;
    } else if (filename.empty() && _loaded) filename = _filename;
    if (!_loaded) init(filename);

    auto in = OIIO::ImageInput::open(filename);
    if (!in) {
        throw std::runtime_error("Failed to open image with OIIO: " + filename);
    }

    std::vector<unsigned char> rawData(_resolution.x * _resolution.y * _numChannels);
    in->read_image(0, 0, 0, _numChannels, OIIO::TypeDesc::UINT8, rawData.data());
    in->close();
    in.reset();

    std::cout << "[Veloxr]" << "Raw data read size=" << rawData.size() << " channels=" << _numChannels << "\n";
    if(!force4Channels) return rawData;

    std::vector<unsigned char> pixelData(_resolution.x * _resolution.y * vsize(4), 255);
    for (uint32_t i = 0; i < _resolution.x * _resolution.y; ++i) {
        for (int c = 0; c < _numChannels && c < 4; ++c) {
            pixelData[i * 4 + c] = rawData[i * _numChannels + c];
        }
    }
    _numChannels = 4;
    std::cout << "[Veloxr]" << "pixelData size=" << pixelData.size() << "\n";
    return pixelData;
}


