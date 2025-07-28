#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>
namespace Veloxr {


    struct Vertex {
        glm::vec4 pos;
        glm::vec4 texCoord;
        int textureUnit;
        int renderUID;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }
        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

            int descriptionIndex = 0;
            attributeDescriptions[descriptionIndex].binding = 0;
            attributeDescriptions[descriptionIndex].location = descriptionIndex;
            attributeDescriptions[descriptionIndex].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[descriptionIndex++].offset = offsetof(Vertex, pos);

            attributeDescriptions[descriptionIndex].binding = 0;
            attributeDescriptions[descriptionIndex].location = descriptionIndex;
            attributeDescriptions[descriptionIndex].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[descriptionIndex++].offset = offsetof(Vertex, texCoord);

            attributeDescriptions[descriptionIndex].binding = 0;
            attributeDescriptions[descriptionIndex].location = descriptionIndex;
            attributeDescriptions[descriptionIndex].format = VK_FORMAT_R32_SINT;
            attributeDescriptions[descriptionIndex++].offset = offsetof(Vertex, textureUnit);

            attributeDescriptions[descriptionIndex].binding = 0;
            attributeDescriptions[descriptionIndex].location = descriptionIndex;
            attributeDescriptions[descriptionIndex].format = VK_FORMAT_R32_SINT;
            attributeDescriptions[descriptionIndex++].offset = offsetof(Vertex, renderUID);

            return attributeDescriptions;
        }
    };

}
