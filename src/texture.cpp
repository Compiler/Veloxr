#include "texture.h"
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

using namespace Veloxr;
OIIO_NAMESPACE_USING  

TextureInfo::TextureInfo(std::string filename) {
    init(filename);
}

void TextureInfo::init(std::string filename) {
    _filename = filename;

    auto in = ImageInput::open(_filename);
    if (!in) {
        std::cerr << "Could not open input: " << _filename << "\n";
    }

    const ImageSpec &in_spec = in->spec();
    _resolution = {(uint32_t)in_spec.width, (uint32_t)in_spec.height};
    _numChannels = in_spec.nchannels;
}
