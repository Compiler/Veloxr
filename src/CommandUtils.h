#pragma once

#include <vulkan/vulkan_core.h>
#include "VLogger.h"

namespace Veloxr {
    class CommandUtils final {
        private:
            CommandUtils() = delete;
            CommandUtils(const CommandUtils&) = delete;
            CommandUtils& operator=(const CommandUtils&) = delete;

            inline static LLogger console{"[Veloxr][CommandUtils] "}; // cpp17 btw

        public:
            static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
            static void endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue graphicsQueue);

    };
}

