#pragma once
#include "Common.h"
#include "RenderEntity.h"
#include "VLogger.h"

namespace Veloxr {


    class VVTexture {
        friend class Veloxr::RenderEntity;
        public:
            VVTexture() = delete;
            VVTexture(VkDevice device);
            ~VVTexture();


        private:
            Veloxr::LLogger console{"[Veloxr][VVTexture] "};


            VkDevice _device;
            VkImage _textureImage;
            VkDeviceMemory _textureImageMemory;
            VkImageView _textureImageView;
            VkSampler _textureSampler;
            int _samplerIndex;

            void destroy() ;
    };

}
