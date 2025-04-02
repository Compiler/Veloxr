#include "device.h"
#include <map>

using namespace Veloxr;

Device::Device(VkInstance instance, VkSurfaceKHR surface, bool enableValidationLayers): _instance(instance), _surface(surface), _enableValidationLayers(enableValidationLayers) {

}


void Device::create() {
    _pickPhysicalDevice();
    _createLogicalDevice();
}

void Device::_createLogicalDevice() {

    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // Backwards compatability with older vulkan
    if (_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();


    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }


    vkGetDeviceQueue(_logicalDevice, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_logicalDevice, indices.presentFamily.value(), 0, &_presentQueue);

    std::cout << "Finished logical device creation! Queue / Present indices: " << indices.graphicsFamily.value() << " " << indices.presentFamily.value() << std::endl;

}


Veloxr::QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) const {
    Veloxr::QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if(indices.isComplete()) break;

        i++;
    }

    return indices;
}
QueueFamilyIndices Device::_findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    std::cout << "Finding device " << device << " queue families\n";

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if(indices.isComplete()) break;

        i++;
    }

    return indices;
}

int Device::_calculateDeviceScore(VkPhysicalDevice device) {
    QueueFamilyIndices indices = _findQueueFamilies(device); 
    if(!indices.isComplete() || !_checkDeviceExtensionSupport(device)) {
        std::cerr << "Device " << device << " failed to support our needs.\n";
        return -1;
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    _maxTextureResolution = deviceProperties.limits.maxImageDimension2D;
    std::cout << "[DEBUG] Physical Device Name: " << deviceProperties.deviceName << "\n";
    std::cout << "[DEBUG] API Version: " << deviceProperties.apiVersion << "\n";
    std::cout << "[DEBUG] Max Texture Resolution: " << _maxTextureResolution << "\n";

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool swapChainAdequate = false;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    if(!swapChainAdequate) {
        std::cerr << "Device " << device << " does not contained swapChain needs.\n";
        return -1;
    }
    std::cout << "Device " << device << " ready to score.\n";


    int score = 0;

    score += deviceProperties.limits.maxImageDimension2D;
    score += deviceFeatures.geometryShader;
    score += deviceFeatures.sparseBinding;
    score += deviceFeatures.tessellationShader;

    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score *= 2;
    }
    std::cout << "Score for device " << deviceProperties.deviceID << " (" << deviceProperties.deviceName << ") = " << score << std::endl;
    return score;
}

bool Device::_checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) const {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void Device::_pickPhysicalDevice() {
    uint32_t deviceCount = 0;

    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> scoreMap;
    scoreMap.insert(std::make_pair(0, VK_NULL_HANDLE));

    for (const auto& device : devices) {
        scoreMap.insert(std::make_pair(_calculateDeviceScore(device), device));
    }

    auto curScore = scoreMap.rbegin()->first;
    if (curScore == -1) {
        std::runtime_error("No suitable device found");
    }
    _physicalDevice = scoreMap.rbegin()->second;
    if (_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}
