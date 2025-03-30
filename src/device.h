#pragma once
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace LRend {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    // Is complete if their are graphics commands supported
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Device {
    private:

        VkInstance _instance;
        VkSurfaceKHR _surface;
        VkPhysicalDevice _device;
        VkQueue _graphicsQueue, _presentQueue;

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device);

        int _calculateDeviceScore(VkPhysicalDevice device);
        bool _checkDeviceExtensionSupport(VkPhysicalDevice device);

        SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device);


    public:
        Device(VkSurfaceKHR surface);
        void create();

}; 
}
