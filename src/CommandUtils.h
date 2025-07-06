#pragma once

#include <vulkan/vulkan_core.h>
#include "VLogger.h"

namespace Veloxr {
    class CommandUtils {
        private:
            LLogger console{"[Veloxr] [CommandUtils] "};

        public:
            VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
            void endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue graphicsQueue);

    };
}

