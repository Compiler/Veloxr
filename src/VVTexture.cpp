#include "VVTexture.h"


namespace Veloxr {

VVTexture::VVTexture(VkDevice device): _device(device) {}

void VVTexture::setDevice(VkDevice device) {
    _device = device;
}


void VVTexture::destroy() {
    if(!_device) {
        console.warn("Called destroy on VVTexture with no device.");
        return;
    }

    console.log("Destroying on device: ", _device);

    if (textureSampler) {
        console.logc1("Destroying Sampler");
        vkDestroySampler(_device, textureSampler, nullptr);
        console.logc1("Destroyed.");
    }

    if ( textureImageView ) {
        console.logc1("Destroying ImageView");
        vkDestroyImageView(_device, textureImageView, nullptr);
        console.logc1("Destroyed.");
    }

    if ( textureImage ) {
        console.logc1("Destroying Image");
        vkDestroyImage(_device, textureImage, nullptr);
        console.logc1("Destroyed.");
    }

    if ( textureImageMemory ) {
        console.logc1("Freeing textureImageMemory");
        vkFreeMemory(_device, textureImageMemory, nullptr);
        console.logc1("Destroyed.");
    }
}

VVTexture::~VVTexture() {
    this->destroy();
}

}
