#include "VVUtils.h"

namespace Veloxr {


    void VVUtils::createBuffer(std::shared_ptr<Veloxr::VVDataPacket> data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        console.logc1(__func__);
        auto device = data->device;
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VVUtils::findMemoryType(data, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }


    uint32_t VVUtils::findMemoryType(std::shared_ptr<Veloxr::VVDataPacket> data, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        console.logc1(__func__);
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(data->physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void VVUtils::copyBuffer(std::shared_ptr<Veloxr::VVDataPacket> data, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        console.logc1(__func__);
        VkCommandBuffer commandBuffer = CommandUtils::beginSingleTimeCommands(data->device, data->commandPool);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        CommandUtils::endSingleTimeCommands(data->device, commandBuffer, data->commandPool, data->graphicsQueue);
    }
}
