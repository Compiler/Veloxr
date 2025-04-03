#include "texture.h"
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <vector>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

OIIOTexture::OIIOTexture(std::string filename) {
    init(filename);
}

void OIIOTexture::init(std::string filename) {
    _filename = filename;

    auto in = ImageInput::open(_filename);
    if (!in) {
        std::cerr << "Could not open input: " << _filename << "\n";
    }

    const ImageSpec &in_spec = in->spec();
    _resolution = {(uint32_t)in_spec.width, (uint32_t)in_spec.height};
    _numChannels = in_spec.nchannels;
    _loaded = true;
}


std::vector<unsigned char> OIIOTexture::load(std::string filename) {
    if(filename.empty() && !_loaded) {
        std::cerr << "You did not initialize your OIIOTexture or did not provide a filename.\n";
        static std::vector<unsigned char> err{};
        return std::move(err);
    }
    if(!_loaded) init(filename);
    auto in = OIIO::ImageInput::open(filename);
    if (!in) {
        throw std::runtime_error("Failed to open image with OIIO: " + filename);
    }

    const OIIO::ImageSpec& spec = in->spec();
    uint32_t width  = spec.width;
    uint32_t height = spec.height;
    int channels    = spec.nchannels;

    std::vector<unsigned char> rawData(width * height * channels);
    in->read_image(0, 0, 0, channels, OIIO::TypeDesc::UINT8, rawData.data());
    in->close();
    in.reset();

    std::vector<unsigned char> pixelData(width * height * 4, 255);
    for (uint32_t i = 0; i < width * height; ++i) {
        for (int c = 0; c < channels && c < 4; ++c) {
            pixelData[i * 4 + c] = rawData[i * channels + c];
        }
    }
    return pixelData; 
}
