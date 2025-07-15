#pragma once

#include <memory>
#include <cstring>
#include <map>

#include "Common.h"
#include "RenderEntity.h"
#include "VVTexture.h"
#include "VVUtils.h"
#include "Vertex.h"

namespace Veloxr {

    class VVShaderStageData {
    
        public:
            VVShaderStageData() = default;
            VVShaderStageData(std::shared_ptr<VVDataPacket> dataPacket);

            void setDataPacket(std::shared_ptr<VVDataPacket> dataPacket);

            // uh do not edit
            void setTextureMap(std::unordered_map<std::string, std::shared_ptr<Veloxr::RenderEntity>>& textureMap);
            void createStageData();
            void updateUniformBuffers(uint32_t currentImage, const Veloxr::UniformBufferObject& ubo);


            VkBuffer& getVertexBuffer() { return vertexBuffer;}
            const std::vector<VkDescriptorSet>& getDescriptorSets() { return descriptorSets; }
            VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout; }


        private:
            inline static LLogger console{"[Veloxr][VVShaderStageData] "}; 
            inline static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
            inline static constexpr uint32_t NUM_VERTICES_PER_TILE = 6;

            std::map<std::string, std::shared_ptr<Veloxr::RenderEntity>>::const_iterator _digestion;

            std::vector<VkBuffer> uniformBuffers;
            std::vector<VkDeviceMemory> uniformBuffersMemory;
            std::vector<void*> uniformBuffersMapped;

            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            VkDescriptorPool descriptorPool;
            std::vector<VkDescriptorSet> descriptorSets;
            VkDescriptorSetLayout descriptorSetLayout;

            void createUniformBuffers();
            void createVertexBuffer();
            void createDescriptorPool();
            void createDescriptorSets();
            void createDescriptorLayout();

            std::shared_ptr<VVDataPacket> _data;
            std::shared_ptr<std::vector<Veloxr::Vertex>> _vertices;
            std::unordered_map<std::string, std::shared_ptr<Veloxr::RenderEntity>> _textureMap;
    };
}
