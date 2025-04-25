#include "renderer.h"

using namespace Veloxr;



void RendererCore::run() {
    init();
    render();
    destroy();
}

void RendererCore::setWindowDimensions(int width, int height) {
    _windowWidth = width;
    _windowHeight = height;
    frameBufferResized = true;
}

void RendererCore::setTextureFilePath(std::string filepath){
    _currentFilepath = filepath;
    destroyTextureData();
    setupTexturePasses();
}


