#pragma once

#include "Common.h"
#include "RenderEntity.h"
#include "VVTexture.h"
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
#include <filesystem>
#include "CommandUtils.h"

// Use direct paths to headers instead of angle bracket includes
#include "OrthographicCamera.h"
#include "DataUtils.h"
#include "VLogger.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Use direct paths to headers instead of angle bracket includes
#include "texture.h"
#include "Vertex.h"
#include "TextureTiling.h"
#include "EntityManager.h"

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
#include "test.h"


#include <cstdint> 
#include <limits> 
#include <algorithm> 
#include <fstream>

#include "device.h"

#ifdef __APPLE__
extern "C" {
    bool checkMetalAvailability();
    const char* getMetalDeviceName();
}
#endif

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif


#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif



#ifdef _WIN32
#include <opencv2/opencv.hpp>
#elif defined(__APPLE__)
#include <opencv4/opencv2/opencv.hpp>
#endif

#define CV_IO_MAX_IMAGE_PIXELS 40536870912

//Platform
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_macos.h>
#include <MetalSurfaceHelper.h>
#include <mach-o/dyld.h>
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif

#ifdef _WIN32
#define PREFIX std::string("C:")
#else
#define PREFIX std::string("/mnt/c")
#endif


// CLIENT SIDE CALLBACKS, IGNORE UNTIL CLASS RENDERCORE
static std::vector<char> readFile(const std::string& filename) {
    std::cout << "[Veloxr]" << "Loading file: " << filename << std::endl;
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


inline const auto LEFT = -1.0f;
inline const auto RIGHT = 1.0f;
inline const auto TOP = -1.0f;
inline const auto BOT = 1.0f;
// TODO:
//      - Alpha layer transparency
//          - Where there are no colors, but there is an alpha layer, fill with alpha checkerboard
//              - Always same size on screen. 
//              - Repeat smallest memory footprint
//      - Return parsable coordinates of what is being viewed.
//          - 




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

//Client side API
public:
    // Structure for passing data to Veloxr

    // Change size of window to render into. 
    void setWindowDimensions(int width, int height); 

    // Initialize the renderer, given a window pointer to render into.
    void init(void* windowHandle = nullptr); 
    void setupGraphics(); 

    // Main introduction to entity handles.
    std::shared_ptr<Veloxr::EntityManager> getEntityManager() { return _entityManager; }

    // Camera API. Use this object to access camera movement related data, zoom, pan, etc.
    Veloxr::OrthographicCamera& getCamera() {
        return _cam;
    }

    // Cleanup calls.
    void destroyTextureData(); 
    void destroy(); 

    // Frame delta for smooth panning
    float deltaMs;
    // For default clients, do not call as an Application with a window handle.
    void run(); 
    void spin(); 
    glm::vec4 _roi {0, 0, 0, 0};
    void resetCrop() {
        _roi = {0, 0, 0, 0};
    }
    void setCrop(glm::vec4 roi) {
        _roi = roi;
    }
    
    // Make drawFrame accessible to external code
    void drawFrame();


    //glm::vec2 getMainEntityPosition()  { }

private: // No client -- internal

    GLFWwindow* window;
    const int WIDTH = 1920;
    const int HEIGHT = 1080;
    int _windowWidth, _windowHeight;
    Veloxr::VeloxrBuffer _currentDataBuffer;
    Veloxr::LLogger console{"[Veloxr][Renderer] "};
    std::shared_ptr<Veloxr::EntityManager> _entityManager;
    // For friend classes / drivers


    VkInstance instance;
    bool noClientWindow = false;

    VkDebugUtilsMessengerEXT debugMessenger;

#ifdef VALIDATION_LAYERS_VALUE
    //bool enableValidationLayers = VALIDATION_LAYERS_VALUE;
    bool enableValidationLayers = true;
#else
    //bool enableValidationLayers = false;
    bool enableValidationLayers = true;
#endif

    Veloxr::OrthographicCamera _cam;


    std::vector<Veloxr::Vertex> vertices = {};
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


    std::shared_ptr<Veloxr::Device> _deviceUtils;
    std::shared_ptr<Veloxr::VVDataPacket> _dataPacket;

    // VK
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
    VkPipelineLayout pipelineLayout; // Uniforms in shaders object
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    uint32_t currentFrame = 0;

    // Structure for holding the VRAM data
    struct VkVirtualTexture {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        int samplerIndex;

        void destroy(VkDevice device) {
            std::cout << "[Veloxr] [Debug] Destroying sampler\n";
            vkDestroySampler(device, textureSampler, nullptr);
            std::cout << "[Veloxr] [Debug] Destroying image view\n";
            vkDestroyImageView(device, textureImageView, nullptr);
            std::cout << "[Veloxr] [Debug] Destroying image \n";
            vkDestroyImage(device, textureImage, nullptr);
            std::cout << "[Veloxr] [Debug] Freeing image memory\n";
            vkFreeMemory(device, textureImageMemory, nullptr);
        }
    };

    std::map<std::string, std::unique_ptr<Veloxr::VVTexture>> _textureMap;
    // Sync
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool frameBufferResized = false;

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
        return GetMetalLayerForNSView(rawHandle);
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
        auto res = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface! Error code: " + std::to_string(res));
        }
        std::cout << "[Veloxr]" << "Windows surface created!\n";
#elif defined(__APPLE__)

            CAMetalLayer* metalLayer = (CAMetalLayer*)GetMetalLayerForNSView(windowHandle);
        VkMetalSurfaceCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        createInfo.pLayer = metalLayer;

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

private:
    // texture utilities ------------------------------------------------
    VkImageView createImageView(VkImage image, VkFormat format);


    // mesh / swapchain helpers ----------------------------------------
    void createVertexBuffer();
    void recreateSwapChain();
    void cleanupSwapChain();
    void createSyncObjects();
    void updateUniformBuffers(uint32_t currentImage);

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

        VkClearValue clearColor = {{{1.0, 0.25f, 0.25f, 1.0f}}};
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

        auto p_shaderStage = _entityManager->getShaderStageData();
        VkBuffer vertexBuffers[] = {p_shaderStage->getVertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &p_shaderStage->getDescriptorSets()[currentFrame], 0, nullptr);

        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);



        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }


    }

    void createCommandBuffer() {
        std::cout << "[Veloxr] Creating command buffers\n";
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
        console.debug(__func__);

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
        // Determine shader path relative to executable
        std::filesystem::path exePath = getExecutablePath();
        // Assumes executable is in 'bin/' and shaders are in 'spirv/' under the same root install prefix
        std::filesystem::path spirvDir = exePath.parent_path().parent_path() / "spirv";

        auto vertShaderCode = readFile((spirvDir / "vert.spv").string());
        auto fragShaderCode = readFile((spirvDir / "frag.spv").string());

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
        rasterizer.cullMode = VK_CULL_MODE_NONE; // TODO: ??
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
        console.debug("!");
        pipelineLayoutInfo.pSetLayouts = &_entityManager->getShaderStageData()->getDescriptorSetLayout();
        console.debug("!!");

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        console.debug("Graphics pipeline Layout created.");


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

        console.debug("Creating graphics pipelines");
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        console.debug("Created graphics pipelines");

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void createImageViews() {

        swapChainImageViews.resize(swapChainImages.size());
        std::cout << "[Veloxr]" << "Creating " << swapChainImageViews.size() << " image views\n";

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

        std::cout << "[Veloxr]" << "Swap Chain Extent chosen: "
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
        std::cout << "[Veloxr]" << "NO client, creating glfw surface\n";
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

        using clock = std::chrono::steady_clock;
        auto fpsTimer = clock::now();
        int frames = 0;
        constexpr auto frameBudget = std::chrono::duration<float>(1.f / 144.0f);
        auto last = clock::now();
        while (!glfwWindowShouldClose(window)) {
            auto now = clock::now();
            auto delta = now - last;
            last = now;

            float dt = std::chrono::duration<float>(delta).count();
            deltaMs = dt;

            glfwPollEvents();
            drawFrame();

            ++frames;

            if (delta < frameBudget) std::this_thread::sleep_for(frameBudget - delta);

            if (clock::now() - fpsTimer > std::chrono::seconds(1)) {
                float fps = frames;
                console.fatal("[Veloxr] ", fps, " FPS  (", 1000.0f / fps, " ms)");
                frames = 0;
                fpsTimer = clock::now();
            }
        }
        
        vkDeviceWaitIdle(device);

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

    void createVulkanInstance() {
        console.log("[Veloxr] enableValidationLayers: ", enableValidationLayers);
        console.log("[Veloxr] checkValidationLayerSupport(): ", checkValidationLayerSupport());
        
#ifdef __APPLE__
        // Check Metal availability using our helper function
        if (!checkMetalAvailability()) {
            std::cerr << "[Veloxr] ERROR: Metal is not available on this system!" << std::endl;
            throw std::runtime_error("Metal is not available on this system");
        }
        const char* deviceName = getMetalDeviceName();
        if (deviceName) {
            std::cout << "[Veloxr] Metal device found: " << deviceName << std::endl;
        }

        // List all available instance extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
        
        std::cout << "\n[Veloxr] Available Vulkan extensions:" << std::endl;
        bool foundMoltenVK = false;
        bool foundPortability = false;
        bool foundMetalSurface = false;
        for (const auto& extension : availableExtensions) {
            std::cout << "[Veloxr]   " << extension.extensionName << std::endl;
            if (strstr(extension.extensionName, "VK_MVK")) {
                foundMoltenVK = true;
            }
            if (strstr(extension.extensionName, "VK_KHR_portability")) {
                foundPortability = true;
            }
            if (strstr(extension.extensionName, "VK_EXT_metal_surface")) {
                foundMetalSurface = true;
            }
        }
        
        std::cout << "\n[Veloxr] Required extension status:" << std::endl;
        std::cout << "[Veloxr]   MoltenVK extensions: " << (foundMoltenVK ? "Found" : "Not found") << std::endl;
        std::cout << "[Veloxr]   Portability enumeration: " << (foundPortability ? "Found" : "Not found") << std::endl;
        std::cout << "[Veloxr]   Metal surface: " << (foundMetalSurface ? "Found" : "Not found") << std::endl;

        if (!foundMoltenVK || !foundPortability || !foundMetalSurface) {
            std::cerr << "[Veloxr] WARNING: Some required extensions are missing!" << std::endl;
        }
#endif

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

        // Start with an empty list of extensions
        std::vector<const char*> requiredExtensions;

#ifdef __APPLE__
        // First, set the portability flag - this is crucial for MoltenVK
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        std::cout << "[Veloxr] Set VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR flag" << std::endl;

        // Add the core extensions needed for MoltenVK
        requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
        std::cout << "[Veloxr] Added core MoltenVK extensions" << std::endl;
#endif

#ifdef _WIN32
        std::cout << "[VELOXR] Setting windows OS flags for surface.\n";
        requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

        // Add GLFW extensions if we're creating our own window
        if(noClientWindow) {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            console.log("[Veloxr] GLFW required extensions count: ", glfwExtensionCount );
            
            for(uint32_t i = 0; i < glfwExtensionCount; i++) {
                console.log("[Veloxr] GLFW extension ", i, ": ", glfwExtensions[i]);
                requiredExtensions.push_back(glfwExtensions[i]);
            }
        }

        // Add validation layer extension if enabled
        if (enableValidationLayers) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Set the final extension list
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        console.log("\n[Veloxr] Total required extensions: ", requiredExtensions.size());
        for(const auto& ext : requiredExtensions) {
            console.log("[Veloxr] Required extension: ", ext);
        }

        // Set up validation layers if enabled
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

        // Create the Vulkan instance
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS) {
            console.fatal("[Veloxr] Failed to create instance with error code: ", result) ;
            if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
                console.fatal("[Veloxr] ERROR: Incompatible driver - Metal might not be available or properly configured");
                console.fatal("[Veloxr] Please ensure MoltenVK is properly installed and configured");
                console.fatal("[Veloxr] Check that:");
                console.fatal("  1. MoltenVK is properly installed via Conan");
                console.fatal("  2. The MoltenVK library is in your library path");
                console.fatal("  3. The Metal framework is properly linked");
            }
            throw std::runtime_error("failed to create instance!");
        }

        console.log("[Veloxr] Created a valid instance!\n");
    }

    // Helper function to get the executable path
    std::filesystem::path getExecutablePath() {
#ifdef _WIN32
        std::vector<wchar_t> pathBuf;
        DWORD copied = 0;
        do {
            pathBuf.resize(pathBuf.size() + MAX_PATH);
            copied = GetModuleFileNameW(NULL, pathBuf.data(), static_cast<DWORD>(pathBuf.size()));
        } while (copied >= pathBuf.size());
        pathBuf.resize(copied);
        return std::filesystem::path(pathBuf.begin(), pathBuf.end());
#elif defined(__linux__)
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        if (count < 0) {
             throw std::runtime_error("Failed to get executable path using readlink");
        }
        return std::filesystem::path(std::string(result, count));
#elif defined(__APPLE__)
        char path[PATH_MAX]; // Using PATH_MAX for consistency
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            return std::filesystem::path(std::string(path));
        } else {
            // Buffer was too small, reallocate and try again
            std::vector<char> pathBuf(size);
            if (_NSGetExecutablePath(pathBuf.data(), &size) == 0) {
                 return std::filesystem::path(std::string(pathBuf.begin(), pathBuf.end()));
            } else {
                 throw std::runtime_error("Failed to get executable path using _NSGetExecutablePath after resize");
            }
        }
#else
        // Placeholder for other platforms or error
        // Returning empty path, might need better error handling or default.
         throw std::runtime_error("Unsupported platform for getExecutablePath");
         return std::filesystem::path(); 
#endif
    }
};

inline void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //printf("Scrolled: x = %.2f, y = %.2f\n", xoffset, yoffset);
    auto app = reinterpret_cast<RendererCore*>(glfwGetWindowUserPointer(window));
    auto& camera = app->getCamera();
    float currentZoom = camera.getZoomLevel();
    float sensitivity = currentZoom * 0.1f;
    camera.zoomToCenter(yoffset*sensitivity);
    std::cout << "[Veloxr] Zoom offset: " << currentZoom << '\n';
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
        glm::vec2 diffs{-dx / app->getCamera().getZoomLevel(), -dy / app->getCamera().getZoomLevel()};
        diffs *= 1000.0 * app->deltaMs;
        app->getCamera().translate(diffs);
        // These will force the FPS to be 7fps on a 3090, there was nothing wrong with my code~!
        /*
        std::cout << "New camera lastx, lasty: " << lastX << ", " << lastY << '\n';
        std::cout << "New camera delta: " << app->deltaMs << '\n';
        std::cout << "New camera diff to be applied: " << diffs.x << " - " << diffs.y <<  '\n';
        std::cout << "New camera roi: " << app->getCamera().getROI().x << ", " << app->getCamera().getROI().y << ", " << app->getCamera().getROI().z << ", " << app->getCamera().getROI().w <<  '\n';
        std::cout << "New camera dims: " << app->getCamera().getWidth() << ", " << app->getCamera().getHeight() <<  '\n';
        std::cout << "New camera zoom: " << app->getCamera().getZoomLevel() <<  '\n';
        std::cout << "New camera x: " << app->getCamera().getPosition().x <<  '\n';
        std::cout << "New camera y: " << app->getCamera().getPosition().y <<  '\n';
        */
    }
}

#define V_USER "ljuek"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    auto setupBuffer = [&](std::string filepath) {
        auto app = reinterpret_cast<RendererCore*>(glfwGetWindowUserPointer(window));
        Veloxr::OIIOTexture texture(filepath);
        Veloxr::VeloxrBuffer buf;
        std::cout << "Moving data...";
        buf.data = texture.load(filepath);
        std::cout << "... done!\n";
        buf.width = texture.getResolution().x;
        buf.height = texture.getResolution().y;
        buf.numChannels = texture.getNumChannels();
        buf.orientation = texture.getOrientation();
    };

    const std::string basePath = std::string("/Users/") + V_USER + "/Downloads/";

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);  
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        setupBuffer(basePath + "fox.jpg");
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        setupBuffer(basePath + "test.png");
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        setupBuffer(basePath + "test2.png");
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        setupBuffer(basePath + "Colonial.jpg");
    }
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        const std::string appendage = std::string(V_USER) == "ljuek" ? "" : "t";
        setupBuffer(basePath + "landscape.tif" + appendage);
    }
    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        setupBuffer(basePath + "56000.jpg");
    }
}

