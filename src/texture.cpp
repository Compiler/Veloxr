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
    OIIO::ImageSpec spec = in->spec();
    auto orientation = spec.get_int_attribute("Orientation", 1);
    std::cout << "Texture Orientation is " << orientation << std::endl;

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

    std::vector<unsigned char> rawData(_resolution.x * _resolution.y * _numChannels);
    in->read_image(0, 0, 0, _numChannels, OIIO::TypeDesc::UINT8, rawData.data());
    in->close();
    in.reset();

    std::vector<unsigned char> pixelData(_resolution.x * _resolution.y * 4, 255);
    for (uint32_t i = 0; i < _resolution.x * _resolution.y; ++i) {
        for (int c = 0; c < _numChannels && c < 4; ++c) {
            pixelData[i * 4 + c] = rawData[i * _numChannels + c];
        }
    }
    _numChannels = 4; // Forcing rgba, lets stick to this.
    return pixelData; 
}
