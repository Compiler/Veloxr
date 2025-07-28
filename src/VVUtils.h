#pragma once
#include <memory>
#include <vulkan/vulkan_core.h>

#include "Common.h"
#include "VLogger.h"
#include "CommandUtils.h"

namespace Veloxr {
    class VVUtils final {
        private:
            VVUtils() = delete;
            VVUtils(const VVUtils&) = delete;
            VVUtils& operator=(const VVUtils&) = delete;

            inline static LLogger console{"[Veloxr][VVUtils] "}; 
        public:

            static void createBuffer(std::shared_ptr<Veloxr::VVDataPacket> data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            static uint32_t findMemoryType(std::shared_ptr<Veloxr::VVDataPacket> data, uint32_t typeFilter, VkMemoryPropertyFlags properties);
            static void copyBuffer(std::shared_ptr<Veloxr::VVDataPacket> data, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    };

}
