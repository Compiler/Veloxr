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
    std::cout << "[Veloxr]" << "Filename=" << _filename << " Orientation=" << orientation << ", resolution" << _resolution.x << "x" << _resolution.y << "\n";
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
    console.fatal("HELP ME: ", rawData.size() * sizeof(unsigned char));
    console.logc1("Ready to read...", filename);
    in->read_image(0, 0, 0, _numChannels, OIIO::TypeDesc::UINT8, &rawData[0]);
    console.logc1("Done reading image.", filename);
    in->close();
    in.reset();
    console.logc1("Done reading image.", filename);
    std::cout << "[Veloxr]" << "Raw data read size=" << rawData.size() << " channels=" << _numChannels << "\n";

    std::vector<unsigned char> pixelData(_resolution.x * _resolution.y * (uint64_t)4, 255);
    for (uint32_t i = 0; i < _resolution.x * _resolution.y; ++i) {
        for (int c = 0; c < _numChannels && c < 4; ++c) {
            pixelData[i * 4 + c] = rawData[i * _numChannels + c];
        }
    }
    _numChannels = 4;
    std::cout << "[Veloxr]" << "pixelData size=" << pixelData.size() << "\n";
    return pixelData;
}


