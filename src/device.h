#pragma once
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Veloxr {

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
        VkPhysicalDevice _physicalDevice;
        VkDevice _logicalDevice;
        VkQueue _graphicsQueue, _presentQueue;
        bool _enableValidationLayers;

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device);

        void _pickPhysicalDevice();
        void _createLogicalDevice();
        int _calculateDeviceScore(VkPhysicalDevice device);
        bool _checkDeviceExtensionSupport(VkPhysicalDevice device);

        SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device);


    public:
        Device(VkSurfaceKHR surface, bool enableValidationLayers = false);
        void create();
        Veloxr::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        VkPhysicalDevice getPhysicalDevice() const { return _physicalDevice; }
        VkDevice getLogicalDevice() const { return _logicalDevice; }

}; 
}
