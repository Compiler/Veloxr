#include "VVShaderStageData.h"

namespace Veloxr {
    
    void setVertices(std::vector<Veloxr::Vertex>& vertices) {

    }

    void VVShaderStageData::createVertexBuffer(std::vector<Veloxr::Vertex>& vertices) {
        console.logc1(__func__);
        console.log("[Veloxr] Creating vertexBuffer\n");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(_data->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        //memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(_data->device, stagingBufferMemory);

        //createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        //copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(_data->device, stagingBuffer, nullptr);
        vkFreeMemory(_data->device, stagingBufferMemory, nullptr);
    }


}
