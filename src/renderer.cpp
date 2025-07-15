#include "renderer.h"
#include "Common.h"
#include "DataUtils.h"
#include "EntityManager.h"
#include <chrono>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using namespace Veloxr;



void RendererCore::run() {
    console.logc1(__func__);
    init();
    render();
    destroy();
}

void RendererCore::spin() {
    console.logc1(__func__);
    render();
    destroy();
}

void RendererCore::setWindowDimensions(int width, int height) {
    console.logc1(__func__);
    _windowWidth = width;
    _windowHeight = height;
    frameBufferResized = true;
}

void RendererCore::init(void* windowHandle) {
    console.logc1(__func__);
    destroy();
    auto now = std::chrono::high_resolution_clock::now();
    auto nowTop = std::chrono::high_resolution_clock::now();

    if(!windowHandle) {
        console.log("Providing window, no client window specified\n");
        initGlfw();
        noClientWindow = true;
    }

    auto timeElapsed = std::chrono::high_resolution_clock::now() - now;
    console.log("Init glfw: ", std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count(), "ms\t", std::chrono::duration_cast<std::chrono::microseconds>(timeElapsed).count(), "microseconds.\n");
    createVulkanInstance();
    setupDebugMessenger();
    if(noClientWindow) createSurface();
    else createSurfaceFromWindowHandle(windowHandle);

    _deviceUtils = std::make_shared<Veloxr::Device>(instance, surface, enableValidationLayers);
    _dataPacket = std::make_shared<Veloxr::VVDataPacket>();
    _deviceUtils->create();
    device = _deviceUtils->getLogicalDevice();
    physicalDevice = _deviceUtils->getPhysicalDevice();
    graphicsQueue = _deviceUtils->getGraphicsQueue();
    presentQueue = _deviceUtils->getPresentationQueue();
    _dataPacket->device = _deviceUtils->getLogicalDevice();
    _dataPacket->physicalDevice = _deviceUtils->getPhysicalDevice();
    _dataPacket->graphicsQueue = graphicsQueue;
    _dataPacket->presentQueue = presentQueue;

    createCommandPool();
    createCommandBuffer();

    _dataPacket->commandPool = commandPool;
    _entityManager = std::make_shared<Veloxr::EntityManager>(_dataPacket);
    console.log("[Veloxr] [Debug] init called and completed. Setting up texture passes from state\n");

    createSwapChain();
    createImageViews();
    createRenderPass();

}

void RendererCore::setupGraphics() {
    createGraphicsPipeline(); // Relies on _entityManager being initialized.
    createFramebuffers();
    createSyncObjects();
    float minX = 0, minY = 0, maxX = 4920, maxY = 2150;
    auto deltaX = std::abs(maxX - minX);
    auto deltaY = std::abs(maxY - minY);
    float offsetX = 0.0f, offsetY = 0.0f;
    if(deltaX > deltaY) {
        offsetY = -deltaY / 2.0f;
    }
    _cam.init(0, maxX - minX, 0, maxY - minY, -1, 1);
    float factor = std::min(_windowWidth, _windowHeight);
    _cam.setZoomLevel(factor / (float)(std::min(deltaX, deltaY)));
    _cam.setProjection(0, _windowWidth, 0, _windowHeight, -1, 1);

}


// ------------- texture utilities ------------------------------------

VkImageView RendererCore::createImageView(VkImage image, VkFormat format) {
    console.logc1(__func__);
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


void RendererCore::recreateSwapChain() {
    console.logc1(__func__);
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
    //TODO: Recalculate what our zoom / focus is
    _cam.setProjection(0, _windowWidth, 0, _windowHeight, -1, 1);

    console.log("[Veloxr] [Debug] Destroying swap chain for recreating swap chain\n");
    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFramebuffers();
}

void RendererCore::cleanupSwapChain() {
    console.logc1(__func__);
    if(device) {
        console.log("[Veloxr] [Debug] Destroying frame buffers\n");
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        }

        console.log("[Veloxr] [Debug] Destroying swapChainImages\n");
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        }

        console.log("[Veloxr] [Debug] Destroying swap chain\n");
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
}

void RendererCore::createSyncObjects() {
    console.logc1(__func__);
    if(imageAvailableSemaphores.size() != 0 && renderFinishedSemaphores.size() != 0) return;
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

// TODO: Just using a dirty bit on camera works for a single image_view. Not all of them. So we need the dirty bit for all imageViews. (triple buffer)
void RendererCore::updateUniformBuffers(uint32_t currentImage) {

    Veloxr::UniformBufferObject ubo{};
    float time = 1;
    ubo.view = _cam.getViewMatrix();
    ubo.proj = _cam.getProjectionMatrix();
    ubo.model = glm::mat4(1.0f);
    ubo.roi = _roi;
    _entityManager->updateUniformBuffers(currentImage, ubo);
}

void RendererCore::destroyTextureData() {
    console.logc1(__func__);
    _currentDataBuffer = {};
    vertices = {};
}

void RendererCore::destroy() {
    console.logc1(__func__);
    if(!device) return;
    console.log("[Veloxr] [Debug] Destroying!", device, "\n");
    cleanupSwapChain();
    destroyTextureData();

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

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

void RendererCore::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || frameBufferResized) {
        console.log("Resizing swapchain\n");
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
