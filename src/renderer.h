#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#define CV_IO_MAX_IMAGE_PIXELS 40536870912
#include <array>
#include <chrono>
#include <glm/ext/matrix_transform.hpp>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <utility>
#include <vulkan/vulkan_core.h>
#include <OrthographicCamera.h>
#include <OrthoCam.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <texture.h>
#include <Vertex.h>
#include <TextureTiling.h>



#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <test.h>


#include <cstdint> 
#include <limits> 
#include <algorithm> 
#include <fstream>

#include <device.h>


#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif


#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

#include <opencv4/opencv2/opencv.hpp>
#define CV_IO_MAX_IMAGE_PIXELS 40536870912

//Platform
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_macos.h>
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif

#ifdef _WIN32
#define PREFIX std::string("C:")
#else
#define PREFIX std::string("/mnt/c")
#endif
static std::vector<char> readFile(const std::string& filename) {
    std::cout << "Loading file: " << filename << std::endl;
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

// Hook extension functions and load them
inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


inline const auto LEFT = -0.9f;
inline const auto RIGHT = 0.9f;
inline const auto TOP = -0.9f;
inline const auto BOT = 0.9f;
// TODO:
//      - Alpha layer transparency
//          - Where there are no colors, but there is an alpha layer, fill with alpha checkerboard
//              - Always same size on screen. 
//              - Repeat smallest memory footprint
//      - Zoom / Pan
//      - Aspect Ratio
//      - Return parsable coordinates of what is being viewed.
//          - 
inline std::vector<Veloxr::Vertex> vertices = {
    {{RIGHT, BOT, 0, 0}, {1.0f, 1.0f, 1, 0}, 0},
    {{LEFT, BOT, 0, 0}, {0.0f, 1.0f, 1, 0}, 0},
    {{LEFT, TOP, 0, 0}, {0.0f, 0.0f, 1, 0}, 0},

    {{LEFT, TOP, 0, 0}, {0.0f, 0.0f, 0, 0}, 0},
    {{RIGHT, TOP, 0, 0}, {1.0f, 0.0f, 0, 0}, 0},
    {{RIGHT, BOT, 0, 0}, {1.0f, 1.0f, 0, 0}, 0},
};


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec4 time;
};

inline bool mousePressed = false;
inline double lastX = 0.0, lastY = 0.0;
inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
inline void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            mousePressed = false;
        }
    }


}

inline void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) ;
inline void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) ;

class RendererCore {
public:

    float deltaMs;
    void run() {
        init();
        render();
        destroy();
    }
    void setWindowDimensions(int width, int height) {
        _windowWidth = width;
        _windowHeight = height;
        frameBufferResized = true;
    }

    /*const*/Veloxr::OrthoCam& getCam() {
        return _cam;
    }

private: // No client

    GLFWwindow* window;
    const int WIDTH = 1920;
    const int HEIGHT = 1080;
    int _windowWidth, _windowHeight;

private: // Client

    VkInstance instance;
    bool noClientWindow = false;

    VkDebugUtilsMessengerEXT debugMessenger;
    bool enableValidationLayers = true;

    Veloxr::OrthoCam _cam;


    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


    std::unique_ptr<Veloxr::Device> _deviceUtils;

    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue, presentQueue;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkColorSpaceKHR swapChainColorSpace;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout; // Uniforms in shaders object
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    uint32_t currentFrame = 0;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    struct VkVirtualTexture {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        Veloxr::OIIOTexture textureData;
        int samplerIndex;

        void destroy(VkDevice device) {
            vkDestroySampler(device, textureSampler, nullptr);
            vkDestroyImageView(device, textureImageView, nullptr);
            vkDestroyImage(device, textureImage, nullptr);
            vkFreeMemory(device, textureImageMemory, nullptr);
        }
    };

    std::map<std::string, VkVirtualTexture> _textureMap;
    // Sync
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool frameBufferResized = false;

private:

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<RendererCore*>(glfwGetWindowUserPointer(window));
        app->frameBufferResized = true;

    }

    void* getWindowHandleFromRaw(void* rawHandle) {
#ifdef __APPLE__
        // mac: we need to ensure we have a CAMetalLayer
        NSView* nsView = (__bridge NSView*)rawHandle;

        if (![nsView layer] || ![nsView.layer isKindOfClass:[CAMetalLayer class]]) {
            [nsView setWantsLayer:YES];
            CAMetalLayer* metalLayer = [CAMetalLayer layer];
            [nsView setLayer:metalLayer];
        }

        return (__bridge void*)nsView.layer;
#else
        return rawHandle;
#endif

    }

    void createSurfaceFromWindowHandle(void* windowHandle) {
#ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = static_cast<HWND>(windowHandle);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        std::cout << "Windows surface created!\n";
#elif defined(__APPLE__)
        VkMetalSurfaceCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        createInfo.pLayer = static_cast<CAMetalLayer*>(windowHandle);

        if (vkCreateMetalSurfaceEXT(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#endif
    }
    void initGlfw() {

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _windowWidth = WIDTH;
        _windowHeight = HEIGHT;
        window = glfwCreateWindow(_windowWidth, _windowHeight, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetKeyCallback(window, key_callback);

    }
public:

    void init(void* windowHandle = nullptr) {
        auto now = std::chrono::high_resolution_clock::now();
        auto nowTop = std::chrono::high_resolution_clock::now();

        if(!windowHandle) {
            std::cout << "Providing window, no client window specified\n";
            initGlfw();
            noClientWindow = true;
        }

        auto timeElapsed = std::chrono::high_resolution_clock::now() - now;
        std::cout << "Init glfw: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count() << "ms\t" << std::chrono::duration_cast<std::chrono::microseconds>(timeElapsed).count() << "microseconds.\n";
        createVulkanInstance();
        setupDebugMessenger();
        if(noClientWindow) createSurface();
        else createSurfaceFromWindowHandle(windowHandle);

        _deviceUtils = std::make_unique<Veloxr::Device>(instance, surface, enableValidationLayers);
        _deviceUtils->create();
        device = _deviceUtils->getLogicalDevice();
        physicalDevice = _deviceUtils->getPhysicalDevice();
        graphicsQueue = _deviceUtils->getGraphicsQueue();
        presentQueue = _deviceUtils->getPresentationQueue();
        createCommandPool();

        now = std::chrono::high_resolution_clock::now();

        //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/very_wide.webp");
        auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/landscape.tif");
        //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/Colonial.jpg");
        //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/56000.jpg");
        //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/landscape1.jpeg");
        //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/landscape2.jpeg");
        std::cout << "Texture creation: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count() << "ms\t" << std::chrono::duration_cast<std::chrono::microseconds>(timeElapsed).count() << "microseconds.\n";
        timeElapsed = std::chrono::high_resolution_clock::now() - now;




        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorLayout();
        createGraphicsPipeline();
        createFramebuffers();
        
        createVertexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffer();
        createSyncObjects();
        auto timeElapsedTop = std::chrono::high_resolution_clock::now() - now;
        std::cout << "Init(): " << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsedTop).count() << "ms\t" << std::chrono::duration_cast<std::chrono::microseconds>(timeElapsedTop).count() << "microseconds.\n";
        _textureMap.clear();
    }
private:

    void addTexture(std::string input_filepath) {
        _textureMap[input_filepath] = {};
        //const auto& [textureImage, textureImageMemory, textureHelper] = createTextureImage(input_filepath);

        //auto imageView = createTextureImageView(textureImage);
        //auto sampler = createTextureSampler();
        //_textureMap[input_filepath].textureImage = textureImage;
        //_textureMap[input_filepath].textureImageMemory = textureImageMemory;
        //_textureMap[input_filepath].textureImageView = imageView;
        //_textureMap[input_filepath].textureSampler = sampler;
        //_textureMap[input_filepath].textureData = textureHelper;
    }

    VkSampler createTextureSampler(std::string input_filepath="") {

        VkSampler textureSampler;
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        // TODO: See if Linear is better for filter
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        // TODO: SEe how to repeat, I think clamp
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

        // TODO: Test with/without this.
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        // false = normalized, true = [0, texWidth], [0, texHeight]
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        // Shadow mapping, ignore.
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;


        // TODO: Mipmap
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;


        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

        return textureSampler;

    
    }

    VkImageView createTextureImageView(VkImage textureImage) {
        return createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    }

    VkImageView createImageView(VkImage image, VkFormat format) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0; // TODO
        barrier.dstAccessMask = 0; // TODO

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage,
                destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
                );

        endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };
        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
                );
        endSingleTimeCommands(commandBuffer);
    }


    std::unordered_map<std::string, VkVirtualTexture> createTiledTexture(std::string input_filepath="") {
        std::unordered_map<std::string, VkVirtualTexture>  result;
        Veloxr::OIIOTexture myTexture{input_filepath};
        _cam.init(0, _windowWidth, 0, _windowHeight, -1, 1);
        Veloxr::TextureTiling tiler{};
        auto maxResolution = _deviceUtils->getMaxTextureResolution();
        std::cout << "Tiling...\n";
        //Veloxr::TiledResult tileData = tiler.tile4(myTexture, maxResolution);
        Veloxr::TiledResult tileData = tiler.tile8(myTexture, maxResolution);
        std::cout << "Done! Bounding box: (" << tileData.boundingBox.x << ", " << tileData.boundingBox.y << ", " << tileData.boundingBox.z << ", " << tileData.boundingBox.w << ") " << std::endl;
        for(const auto& [indx, tileData] : tileData.tiles){
            VkVirtualTexture tileTexture;
            int texWidth    = tileData.width;
            int texHeight   = tileData.height;
            int texChannels = 4;//myTexture.getNumChannels();

            std::cout << "HELP MY CHANNELS ARE " << myTexture.getNumChannels() << std::endl;
            VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * 
                static_cast<VkDeviceSize>(texHeight) *
                static_cast<VkDeviceSize>(texChannels);

            std::cout << "Loading texture of size " 
                << texWidth << " x " << texHeight << ": " 
                << (imageSize / 1024.0 / 1024.0) << " MB" << std::endl;


            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            //memcpy(data, res.begin()->pixelData.data()/*myTexture.load(input_filepath).data()*/, static_cast<size_t>(imageSize));
            memcpy(data, tileData.pixelData.data()/*myTexture.load(input_filepath).data()*/, static_cast<size_t>(imageSize));
            vkUnmapMemory(device, stagingBufferMemory);

            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
            createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
            tileTexture.textureImage = textureImage;
            tileTexture.textureImageMemory = textureImageMemory;
            tileTexture.textureData = myTexture;

            
            auto imageView = createTextureImageView(textureImage);
            auto sampler = createTextureSampler();
            tileTexture.textureImageView = imageView;
            tileTexture.textureSampler = sampler;
            tileTexture.samplerIndex = indx;

            _textureMap[input_filepath + "_tile_" + std::to_string(indx)] = tileTexture;
        }
        vertices = std::vector<Veloxr::Vertex>(tileData.vertices.begin(), tileData.vertices.end());
        for(Veloxr::Vertex& vertice : vertices) {
            const auto& [position, texCoords, texUnit] = vertice;
            std::cout << "[" << position.x << ", " << position.y << "]\t\t|\t\t[" << texCoords.x << ", " << texCoords.y << "]\t|\t" << texUnit << std::endl;
        }
        float minX = +9999.0f, maxX = -9999.0f;
        float minY = +9999.0f, maxY = -9999.0f;
        for (auto &v : vertices) {
            minX = std::min(minX, v.pos.x);
            maxX = std::max(maxX, v.pos.x);
            minY = std::min(minY, v.pos.y);
            maxY = std::max(maxY, v.pos.y);
        }
        std::cout << "Final geometry bounding box: X in [" 
            << minX << ", " << maxX << "], Y in [" 
            << minY << ", " << maxY << "]\n";
        //_cam.setPosition({(maxX - minX) / 2.0f, (maxY - minY) / 2.0f});
        auto deltaX = std::abs(maxX - minX);
        auto deltaY = std::abs(maxY - minY);
        float offsetX = 0.0f, offsetY = 0.0f;
        if(deltaX > deltaY) {
            offsetY = -deltaY / 2.0f;
        }
        _cam.setPosition({offsetX, offsetY});
        _cam.fitViewport(minX, maxX, minY, maxY);

        return {};
    }


    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    void createDescriptorSets() {

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
                imageInfo.imageView = structure.textureImageView;
                imageInfo.sampler = structure.textureSampler;
                orderedSamplers[structure.samplerIndex] = (imageInfo);
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

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(32 * MAX_FRAMES_IN_FLIGHT);//static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * _textureMap.size());// static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }


    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void createDescriptorLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 32;//static_cast<uint32_t>(_textureMap.size());
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
          VkCommandBuffer commandBuffer = beginSingleTimeCommands();

          VkBufferCopy copyRegion{};
          copyRegion.size = size;
          vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

          endSingleTimeCommands(commandBuffer);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

     void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void recreateSwapChain() {
        // TODO: This is client only
        int width = 0, height = 0;
        if(noClientWindow) {
            glfwGetFramebufferSize(window, &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }
            _windowWidth = width;
            _windowHeight = height;
        }
        vkDeviceWaitIdle(device);
        _cam.setProjection(0, _windowWidth, 0, _windowHeight, -1, 1);


        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createFramebuffers();
    }

    void cleanupSwapChain() {
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        }

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);

    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled since we block on drawFrame :/

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                    vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }


    void updateUniformBuffers(uint32_t currentImage) {
        UniformBufferObject ubo{};
        float time = 1;
        ubo.view = _cam.getViewMatrix();
        ubo.proj = _cam.getProjectionMatrix();
        ubo.model = glm::mat4(1.0f);
        //ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0,0,1));
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

public:
    void drawFrame() {
        //std::cout << "Drawing frame with extent: " << swapChainExtent.width << "x" << swapChainExtent.height << std::endl;
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;

        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || frameBufferResized) {
            std::cout << "Resizing swapchain\n";
            frameBufferResized = false;
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        vkResetCommandBuffer(commandBuffers[currentFrame],  0);

        updateUniformBuffers(currentFrame);

        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(presentQueue, &presentInfo);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    }
private:

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {{{0.25f, 0.25f, 0.25f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);



        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }


    }

    void createCommandBuffer() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

    }

    void createCommandPool() {
        Veloxr::QueueFamilyIndices queueFamilyIndices = _deviceUtils->findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

    }

    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // Subpass
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;// layout(location=0) out vec4 outColor
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef; 

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

    }


    VkShaderModule createShaderModule(const std::vector<char>& code) {

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    // Immutable.
    void createGraphicsPipeline() {
        auto vertShaderCode = readFile(std::string(PROJECT_ROOT_DIR) + "/spirv/vert.spv");
        auto fragShaderCode = readFile(std::string(PROJECT_ROOT_DIR) + "/spirv/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);


        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // TODO LR: RTX :eyes: ?
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main"; // Can use same shader with multiple entry points for one file n execution??
        vertShaderStageInfo.pSpecializationInfo = nullptr; // Assign pipelinecreation-time-constants for shaders

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        auto bindingDescription = Veloxr::Vertex::getBindingDescription();
        auto attributeDescriptions = Veloxr::Vertex::getAttributeDescriptions();


        // Hardcoded in shader for now :D
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // Default, triangles
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Render to the extents of our viewport
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;


        // Mutable properties must be explicitly defined for a graphics pipeline, use viewport and scissor for qml window size updates
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();


        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; 
        rasterizer.depthBiasClamp = 0.0f; 
        rasterizer.depthBiasSlopeFactor = 0.0f; 

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; 
        multisampling.pSampleMask = nullptr; 
        multisampling.alphaToCoverageEnable = VK_FALSE; 
        multisampling.alphaToOneEnable = VK_FALSE; 

        // Blend render passes into same fragment section in framebuffer :D
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; 
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; 
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional


        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void createImageViews() {

        swapChainImageViews.resize(swapChainImages.size());
        std::cout << "Creating " << swapChainImageViews.size() << " image views\n";

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);
        }

    }

    void createSwapChain() {
        Veloxr::SwapChainSupportDetails swapChainSupport = _deviceUtils->querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1; //stereoscopic, ignore.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        Veloxr::QueueFamilyIndices indices = _deviceUtils->findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        // Store member meta data for swapchain
        swapChainImageFormat = surfaceFormat.format;
        swapChainColorSpace = surfaceFormat.colorSpace;
        swapChainExtent = extent;

        std::cout << "Swap Chain Extent chosen: "
          << swapChainExtent.width << " x " 
          << swapChainExtent.height << std::endl;

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; 
            createInfo.pQueueFamilyIndices = nullptr; 
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        // Need to resize, we specified the minimum not the actual.
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());



    }

    void createSurface() {
        std::cout << "NO client, creating glfw surface\n";
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];

    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            // Triple buffering here
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        // Definitely supported vvv
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            // TODO: This is user side, need client side passthrough
            int width = _windowWidth, height = _windowHeight;
            if(noClientWindow) {
                glfwGetFramebufferSize(window, &width, &height);
            }

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }

    }

    void render() {

        auto timer = std::chrono::high_resolution_clock::now();
        int frames = 0;
        
        while (!glfwWindowShouldClose(window)) {
            auto now = std::chrono::high_resolution_clock::now();
            glfwPollEvents();
            drawFrame();

            auto timeElapsed = std::chrono::high_resolution_clock::now() - now;

            auto timerElapsed = std::chrono::high_resolution_clock::now() - timer;
            deltaMs = std::chrono::duration<float>(timeElapsed).count();
            auto elapsedPerSec = std::chrono::duration_cast<std::chrono::milliseconds>(timerElapsed).count();
            if(elapsedPerSec > 1000) {
                
                std::cout << "Time elapsed single frame: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count() << "ms\t" << std::chrono::duration_cast<std::chrono::microseconds>(timeElapsed).count() << "microseconds.\n";
                timer = std::chrono::high_resolution_clock::now();
                std::cout << frames << " FPS, " << 1000.0f/frames << " ms.\n";
                frames = 0;
            }
            frames++;
        }

        vkDeviceWaitIdle(device);

    }

public:
    void destroy() {

        cleanupSwapChain();

        for(auto& [name, data] : _textureMap) data.destroy(device);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
private:

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    void setupDebugMessenger() {
        if(!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }

    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;

    }

    // Won't need this in library. Pass in the vulkan instance. (Addendum) I am wrong.
    void createVulkanInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "ImageRenderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = "Cast";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        // ifdef macos / molten
        std::vector<const char*> requiredExtensions;
        if(noClientWindow) {
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            createInfo.enabledExtensionCount = glfwExtensionCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;
            createInfo.enabledLayerCount = 0;


            for(uint32_t i = 0; i < glfwExtensionCount; i++) {
                requiredExtensions.emplace_back(glfwExtensions[i]);
            }

            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        }

        requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);  // This is the critical addition!

#ifdef __APPLE__
        requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
        requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

        if (enableValidationLayers) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

#ifdef __APPLE__
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

        std::cout << "Created a valid instance!\n";
    }
};

inline void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //printf("Scrolled: x = %.2f, y = %.2f\n", xoffset, yoffset);
    auto app = reinterpret_cast<RendererCore*>(glfwGetWindowUserPointer(window));
    auto& camera = app->getCam();
    float currentZoom = camera.getZoomLevel();
    float sensitivity = currentZoom * 0.1f;
    camera.zoomToCenter(yoffset*sensitivity);
    //camera.addToZoom(yoffset * sensitivity);
}

inline void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (mousePressed) {
        auto app = reinterpret_cast<RendererCore*>(glfwGetWindowUserPointer(window));
        double dx = xpos - lastX;
        double dy = ypos - lastY;

     //   printf("Dragging: dx = %.2f, dy = %.2f\n", dx, dy);

        lastX = xpos;
        lastY = ypos;
        glm::vec2 diffs{-dx * 20.0f, -dy * 20.0f};
        diffs *= app->getCam().getZoomLevel() * 400 * app->deltaMs;
        app->getCam().translate(diffs);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);  
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        auto app = reinterpret_cast<RendererCore*>(glfwGetWindowUserPointer(window));
    }

}

