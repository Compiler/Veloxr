#pragma once
#include "Common.h"
#include "RenderEntity.h"
#include "VLogger.h"

namespace Veloxr {


    class VVTexture {
        //friend class Veloxr::RenderEntity;
        public:
            VVTexture() = default;

            VVTexture(VkDevice device);

            void setDevice(VkDevice device);

            // Very exposed. This might as well be a Struct.
            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
            VkImageView textureImageView;
            VkSampler textureSampler;
            int samplerIndex;

            ~VVTexture();

        private:
            Veloxr::LLogger console{"[Veloxr][VVTexture] "};


            VkDevice _device;

            void destroy();
    };
}
