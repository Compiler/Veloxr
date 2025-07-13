#include "VVTexture.h"

using Veloxr::VVTexture;


VVTexture::VVTexture(VkDevice device): _device(device) {}
VVTexture::~VVTexture() {
    this->destroy();
}


void VVTexture::destroy() {
    console.logc3("Destroying Sampler");
    vkDestroySampler(_device, _textureSampler, nullptr);

    console.logc3("Destroing ImageView");
    vkDestroyImageView(_device, _textureImageView, nullptr);

    console.logc3("Destroing Image");
    vkDestroyImage(_device, _textureImage, nullptr);

    console.logc3("Freeing textureImageMemory");
    vkFreeMemory(_device, _textureImageMemory, nullptr);
}
