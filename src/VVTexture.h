#pragma once
#include "Common.h"
#include "TileManager.h"
#include "DataUtils.h"
#include "TextureTiling.h"
#include "VLogger.h"
#include "CommandUtils.h"
#include "Vertex.h"
#include <memory>

namespace Veloxr {

    struct VVTileData{
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        uint32_t samplerIndex;
    };

    class VVTexture {
        public:
            VVTexture() = default;

            VVTexture(std::shared_ptr<VVDataPacket> dataPacket);
            void setDataPacket(std::shared_ptr<VVDataPacket> dataPacket);

            void tileTexture(std::shared_ptr<Veloxr::VeloxrBuffer> buffer);

            // Very exposed. This might as well be a Struct.
            const std::vector<Veloxr::VVTileData>& getTiledResult() const { return _tiledResult; }

            // Vertices are in base coordinates. Spawning at 0,0 and spanning width / height
            [[nodiscard]] std::vector<Veloxr::Vertex>& getVertices() { return _vertices; };
            const glm::vec4& getBoundingBox() const { return _currentBoundingBox; }

            ~VVTexture();

        private:
            Veloxr::LLogger console{"[Veloxr][VVTexture] "};

            static Veloxr::TileManager _tileManager;

            std::shared_ptr<VVDataPacket> _data;
            std::vector<Veloxr::Vertex> _vertices;
            std::vector<Veloxr::VVTileData> _tiledResult{};

            glm::vec4 _currentBoundingBox;

            void destroy();

            void createImage(uint32_t width, uint32_t height, VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkImage& image, VkDeviceMemory& imageMemory) ;

            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

            void copyBufferToImage(VkBuffer buffer, VkImage image,
                    uint32_t width, uint32_t height);

            VkSampler  createTextureSampler();
            VkImageView createTextureImageView(VkImage textureImage);
            VkImageView createImageView(VkImage image, VkFormat format);
    };
}
