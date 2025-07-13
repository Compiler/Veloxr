#pragma once
#include "Common.h"
#include "DataUtils.h"
#include "VLogger.h"
#include "CommandUtils.h"
#include "Vertex.h"
#include <memory>

namespace Veloxr {


    class VVTexture {
        public:
            VVTexture() = default;

            VVTexture(std::shared_ptr<VVDataPacket> dataPacket);
            void setDataPacket(std::shared_ptr<VVDataPacket> dataPacket);

            void tileTexture(std::shared_ptr<Veloxr::VeloxrBuffer> buffer);

            // Very exposed. This might as well be a Struct.
            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
            VkImageView textureImageView;
            VkSampler textureSampler;
            int samplerIndex;

            [[nodiscard]] std::vector<Veloxr::Vertex>& getVertices() { return _vertices; };

            ~VVTexture();

        private:
            Veloxr::LLogger console{"[Veloxr][VVTexture] "};

            std::shared_ptr<VVDataPacket> _data;
            std::vector<Veloxr::Vertex> _vertices;

            void destroy();

            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

            void createImage(uint32_t width, uint32_t height, VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkImage& image, VkDeviceMemory& imageMemory) ;

            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer& buffer,
                    VkDeviceMemory& bufferMemory);

            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

            void copyBufferToImage(VkBuffer buffer, VkImage image,
                    uint32_t width, uint32_t height);

            VkSampler  createTextureSampler();
            VkImageView createTextureImageView(VkImage textureImage);
            VkImageView createImageView(VkImage image, VkFormat format);
    };
}
