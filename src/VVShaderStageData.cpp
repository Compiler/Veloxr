#include "VVShaderStageData.h"
#include "Common.h"
#include "EntityManager.h"
#include <stdexcept>

namespace Veloxr {
    
    VVShaderStageData::VVShaderStageData(std::shared_ptr<VVDataPacket> dataPacket): _data(dataPacket) {

        

    }

    void VVShaderStageData::setDataPacket(std::shared_ptr<VVDataPacket> dataPacket) {
        _data = dataPacket;
    }

    void VVShaderStageData::setTextureMap(std::unordered_map<std::string, std::shared_ptr<Veloxr::RenderEntity>>& textureMap) {
        _textureMap = textureMap;

        for( auto& [_, entity] : _textureMap) {
            _vertices->insert(_vertices->begin(), entity->getVertices().begin(), entity->getVertices().end());
        }
    }
    void VVShaderStageData::createStageData() {
        if (!_vertices.get() || _vertices->empty()) {
            console.fatal("Cannot create stage data for empty vertices.");
            throw std::runtime_error("Cannot create stage data with no entities.");
            return;
        }

        createUniformBuffers();
        createVertexBuffer();
        createDescriptorPool();
        createDescriptorSets();
        createDescriptorLayout();
    }

    void VVShaderStageData::createVertexBuffer() {
        console.logc1(__func__);
        console.log("[Veloxr] Creating vertexBuffer\n");
        VkDeviceSize bufferSize = sizeof(_vertices->front()) * _vertices->size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VVUtils::createBuffer(_data, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(_data->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, _vertices->data(), (size_t) bufferSize);
        vkUnmapMemory(_data->device, stagingBufferMemory);

        VVUtils::createBuffer(_data, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        VVUtils::copyBuffer(_data, stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(_data->device, stagingBuffer, nullptr);
        vkFreeMemory(_data->device, stagingBufferMemory, nullptr);
    }

    void VVShaderStageData::createDescriptorPool() {
        // ASSERT -> WE HAVE TILED OUR TEXTURE
        console.logc1(__func__);
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(_vertices->size() / NUM_VERTICES_PER_TILE * MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(_data->device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void VVShaderStageData::createUniformBuffers() {
        console.logc1(__func__);
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VVUtils::createBuffer(_data, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(_data->device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }

    }

    void VVShaderStageData::createDescriptorSets() {
        console.logc1(__func__);

        auto device = _data->device;

        console.log("[Veloxr] Creating descriptor sets for ", MAX_FRAMES_IN_FLIGHT, " frames \n");
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        console.log("[Veloxr] Allocated new descriptor sets\n");


        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::vector<VkDescriptorImageInfo> imageInfos;
            std::map<int, VkDescriptorImageInfo> orderedSamplers;
            for (auto& [filepath, structure] : _textureMap) {
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = structure->getVVTexture().textureImageView;
                imageInfo.sampler = structure->getVVTexture().textureSampler;
                orderedSamplers[structure->getVVTexture().samplerIndex] = (imageInfo);
            }

            for(auto& [_, imageInfo] : orderedSamplers) {
                imageInfos.push_back(imageInfo);
            }

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = static_cast<uint32_t>(imageInfos.size());
            descriptorWrites[1].pImageInfo = imageInfos.data();

            console.log("[Veloxr] Updating descriptor sets\n");
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }


    void VVShaderStageData::createDescriptorLayout() {
        console.logc1(__func__);
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(_data->physicalDevice, &deviceProperties);

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = std::min((uint32_t)2048, deviceProperties.limits.maxPerStageDescriptorSamplers);
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(_data->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void VVShaderStageData::updateUniformBuffers(uint32_t currentImage, const Veloxr::UniformBufferObject& ubo) {
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

}
