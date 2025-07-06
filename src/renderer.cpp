#include "renderer.h"
#include <stdexcept>

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

void RendererCore::setTextureFilePath(std::string filepath){
    console.logc1(__func__);
    if(!device) {
        console.log("[Veloxr] Updating texture filepath\n");
        _currentFilepath = filepath;
        return;
    }
    console.log("[Veloxr] Updating texture filepath and destroying\n");
    destroyTextureData();
    _currentFilepath = filepath;
    setupTexturePasses();
}

void RendererCore::setTextureBuffer(Veloxr::VeloxrBuffer&& buffer){
    console.logc1(__func__);
    if (!device ) {
        console.log("[Veloxr] Updating texture buffer \n");
        _currentDataBuffer = buffer;
        return;
    }
    console.log("[Veloxr] Updating texture buffer filepath and destroying\n");
    destroyTextureData();
    _currentDataBuffer = buffer;
    setupTexturePasses();
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

    _deviceUtils = std::make_unique<Veloxr::Device>(instance, surface, enableValidationLayers);
    _deviceUtils->create();
    device = _deviceUtils->getLogicalDevice();
    physicalDevice = _deviceUtils->getPhysicalDevice();
    graphicsQueue = _deviceUtils->getGraphicsQueue();
    presentQueue = _deviceUtils->getPresentationQueue();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createUniformBuffers();
    createCommandPool();
    if(!_currentFilepath.empty() || !_currentDataBuffer.data.empty()) {
        console.log("[Veloxr] [Debug] init called and completed. Setting up texture passes from state\n");
        setupTexturePasses();
    }
}


void RendererCore::setupTexturePasses() {
    console.logc1(__func__);
    if(!device) {
        console.log("[Veloxr] [Debug] [Warn] No device instantiated. Returning early. Do not call setupTexturePasses without setting a filepath or buffer.\n");
        return;
    }
    console.log("[Veloxr] [Debug] setting up texture pass.\n");

    auto now = std::chrono::high_resolution_clock::now();
    auto nowTop = std::chrono::high_resolution_clock::now();
    now = std::chrono::high_resolution_clock::now();

    //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/very_wide.webp");
    std::unordered_map<std::string, RendererCore::VkVirtualTexture> res; 
    if (_currentDataBuffer.data.empty() == false) {
        res = createTiledTexture(std::move(_currentDataBuffer));
    } else if (_currentFilepath.empty() == false) {
        res = createTiledTexture(_currentFilepath);
    } else {
        throw std::runtime_error("[Veloxr] setupTexturePasses called with no valid filepath or data buffer.\n");
    }
    //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/Colonial.jpg");
    //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/56000.jpg");
    //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/landscape1.jpeg");
    //auto res = createTiledTexture(PREFIX+"/Users/ljuek/Downloads/landscape2.jpeg");
    auto timeElapsed = std::chrono::high_resolution_clock::now() - now;
    console.log("Texture creation: ", std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count(), "ms\t", std::chrono::duration_cast<std::chrono::microseconds>(timeElapsed).count(), "microseconds.\n");


    createVertexBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffer();
    createSyncObjects();
    auto timeElapsedTop = std::chrono::high_resolution_clock::now() - now;
    console.log("Init(): ", std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsedTop).count(), "ms\t", std::chrono::duration_cast<std::chrono::microseconds>(timeElapsedTop).count(), "microseconds.\n");
    _textureMap.clear();

}

// ------------- texture utilities ------------------------------------

VkSampler RendererCore::createTextureSampler(std::string input_filepath) {
    console.logc1(__func__);

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

    // TODO: Test with/without this.createGraphicsPipeline
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

VkImageView RendererCore::createTextureImageView(VkImage textureImage) {
    console.logc1(__func__);
    return createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

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

// ------------- image-layout / copy helpers --------------------------
void RendererCore::transitionImageLayout(VkImage image, VkFormat format,
                                     VkImageLayout oldLayout, VkImageLayout newLayout) {
    console.logc1(__func__);
    VkCommandBuffer commandBuffer = commandUtils.beginSingleTimeCommands(device, commandPool);

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

    commandUtils.endSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}

void RendererCore::copyBufferToImage(VkBuffer buffer, VkImage image,
                                 uint32_t width, uint32_t height) {
    console.logc1(__func__);
    VkCommandBuffer commandBuffer = commandUtils.beginSingleTimeCommands(device, commandPool);

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
    commandUtils.endSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}

// ------------- tiling / upload path ---------------------------------
//

std::unordered_map<std::string, RendererCore::VkVirtualTexture> RendererCore::createTiledTexture(Veloxr::VeloxrBuffer&& buffer) {
    console.logc1(__func__);
    std::unordered_map<std::string, VkVirtualTexture>  result;
    //_cam.init(0, _windowWidth, 0, _windowHeight, -1, 1);
    Veloxr::TextureTiling tiler{};
    auto maxResolution = _deviceUtils->getMaxTextureResolution();
    console.log("[Veloxr]", "Not Tiling...\n");

    VkVirtualTexture tileTexture;
    int texWidth    = buffer.width;
    int texHeight   = buffer.height;
    int texChannels = 4;//buffer.numChannels;//myTexture.getNumChannels();

    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * 
        static_cast<VkDeviceSize>(texHeight) *
        static_cast<VkDeviceSize>(texChannels);

    console.log("[Veloxr]", "Loading texture of size ", texWidth, " x ", texHeight, ": ", (imageSize / 1024.0 / 1024.0), " MB");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, buffer.data.data(), static_cast<size_t>(imageSize));
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


    auto imageView = createTextureImageView(textureImage);
    auto sampler = createTextureSampler();
    tileTexture.textureImageView = imageView;
    tileTexture.textureSampler = sampler;
    tileTexture.samplerIndex = 0;

    _textureMap["buffer"] = tileTexture;
    //vertices = std::vector<Veloxr::Vertex>(tileData.vertices.begin(), tileData.vertices.end());
    for(Veloxr::Vertex& vertice : vertices) {
        const auto& [position, texCoords, texUnit] = vertice;
        console.log("[Veloxr]", "[", position.x, ", ", position.y, "]\t\t|\t\t[", texCoords.x, ", ", texCoords.y, "]\t|\t", texUnit);
    }
    float minX = +9999.0f, maxX = -9999.0f;
    float minY = +9999.0f, maxY = -9999.0f;
    for (auto &v : vertices) {
        minX = std::min(minX, v.pos.x);
        maxX = std::max(maxX, v.pos.x);
        minY = std::min(minY, v.pos.y);
        maxY = std::max(maxY, v.pos.y);
    }
    console.log("[Veloxr]", "Final geometry bounding box: X in [", minX, ", ", maxX, "], Y in [", minY, ", ", maxY, "]");
    //_cam.setPosition({(maxX - minX) / 2.0f, (maxY - minY) / 2.0f});
    auto deltaX = std::abs(maxX - minX);
    auto deltaY = std::abs(maxY - minY);
    float offsetX = 0.0f, offsetY = 0.0f;
    if(deltaX > deltaY) {
        offsetY = -deltaY / 2.0f;
    }
    _cam.init(0, maxX - minX, 0, maxY - minY, -1, 1);
    //_cam.setPosition({offsetX, offsetY});
    float firstZoom =  deltaX / _cam.getWidth(); 
    float secondZoom = deltaY / _cam.getHeight(); 
    console.fatal("Zoom changes: ", firstZoom, secondZoom);
    //_cam.setZoomLevel(std::max(firstZoom, secondZoom));
    _cam.setProjection(0, _windowWidth, 0, _windowHeight, -1, 1);

    return {};
}

std::unordered_map<std::string, RendererCore::VkVirtualTexture> RendererCore::createTiledTexture(std::string input_filepath) {
    console.logc1(__func__);
    std::unordered_map<std::string, VkVirtualTexture>  result;
    Veloxr::OIIOTexture myTexture{input_filepath};
    Veloxr::TextureTiling tiler{};
    auto maxResolution = _deviceUtils->getMaxTextureResolution();
    console.log("[Veloxr]", "Tiling...");
    //Veloxr::TiledResult tileData = tiler.tile4(myTexture, maxResolution);
    Veloxr::TiledResult tileData = tiler.tile8(myTexture, maxResolution);
    console.log("[Veloxr]", "Done! Bounding box: (", tileData.boundingBox.x, ", ", tileData.boundingBox.y, ", ", tileData.boundingBox.z, ", ", tileData.boundingBox.w, ") ");
    for(const auto& [indx, tileData] : tileData.tiles){
        VkVirtualTexture tileTexture;
        int texWidth    = tileData.width;
        int texHeight   = tileData.height;
        int texChannels = 4;//myTexture.getNumChannels();

        console.log("[Veloxr]", "HELP MY CHANNELS ARE ", myTexture.getNumChannels());
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * 
            static_cast<VkDeviceSize>(texHeight) *
            static_cast<VkDeviceSize>(texChannels);

        console.log("[Veloxr]", "Loading texture of size ", texWidth, " x ", texHeight, ": ", (imageSize / 1024.0 / 1024.0), " MB");

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
        console.log("[Veloxr]", "[", position.x, ", ", position.y, "]\t\t|\t\t[", texCoords.x, ", ", texCoords.y, "]\t|\t", texUnit);
    }
    float minX = +9999.0f, maxX = -9999.0f;
    float minY = +9999.0f, maxY = -9999.0f;
    for (auto &v : vertices) {
        minX = std::min(minX, v.pos.x);
        maxX = std::max(maxX, v.pos.x);
        minY = std::min(minY, v.pos.y);
        maxY = std::max(maxY, v.pos.y);
    }
    console.log("[Veloxr]", "Final geometry bounding box: X in [", minX, ", ", maxX, "], Y in [", minY, ", ", maxY, "]");
    auto deltaX = std::abs(maxX - minX);
    auto deltaY = std::abs(maxY - minY);
    float offsetX = 0.0f, offsetY = 0.0f;
    if(deltaX > deltaY) {
        offsetY = -deltaY / 2.0f;
    }
    _cam.init(0, maxX - minX, 0, maxY - minY, -1, 1);
    float factor = std::min(_windowWidth, _windowHeight);
    console.fatal("Zoom changes: ", " - ",  " - ", _cam.getWidth(), " - ", _cam.getHeight());
    _cam.setZoomLevel(factor / (float)(std::min(deltaX, deltaY)));
    _cam.setProjection(0, _windowWidth, 0, _windowHeight, -1, 1);

    return {};
}

// ------------- resource creation helpers ---------------------------
void RendererCore::createImage(uint32_t width, uint32_t height, VkFormat format,
                           VkImageTiling tiling, VkImageUsageFlags usage,
                           VkMemoryPropertyFlags properties,
                           VkImage& image, VkDeviceMemory& imageMemory) {
    console.logc1(__func__);
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

void RendererCore::createDescriptorSets() {
    console.logc1(__func__);

    console.log("[Veloxr] Creating descriptor sets for ", MAX_FRAMES_IN_FLIGHT, " frames \n");
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
    console.log("[Veloxr] Allocated new descriptor sets\n");


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

        console.log("[Veloxr] Updating descriptor sets\n");
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void RendererCore::createDescriptorPool() {
    // ASSERT -> WE HAVE TILED OUR TEXTURE
    console.logc1(__func__);
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(_textureMap.size() * MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void RendererCore::createUniformBuffers() {
    console.logc1(__func__);
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void RendererCore::createDescriptorLayout() {
    console.logc1(__func__);
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    console.warn("Texture map size for descriptor layout: ", _textureMap.size());
    samplerLayoutBinding.descriptorCount = 32;//std::min((decltype(_textureMap.size()))1, _textureMap.size());
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

void RendererCore::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkBuffer& buffer,
                            VkDeviceMemory& bufferMemory) {
    console.logc1(__func__);
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

void RendererCore::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    console.logc1(__func__);
      VkCommandBuffer commandBuffer = commandUtils.beginSingleTimeCommands(device, commandPool);

      VkBufferCopy copyRegion{};
      copyRegion.size = size;
      vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

      commandUtils.endSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}

uint32_t RendererCore::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    console.logc1(__func__);
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void RendererCore::createVertexBuffer() {
    console.logc1(__func__);
    console.log("[Veloxr] Creating vertexBuffer\n");
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
    //console.logc1(__func__);
    static std::unordered_map<uint32_t, bool> _dirtyMap;
    auto curDirtyStatus = _cam.getDirty();
    
    _cam.resetDirty();
    UniformBufferObject ubo{};
    float time = 1;
    ubo.view = _cam.getViewMatrix();
    ubo.proj = _cam.getProjectionMatrix();
    ubo.model = glm::mat4(1.0f);
    ubo.roi = _roi;
    //ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0,0,1));
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void RendererCore::destroyTextureData() {
    console.logc1(__func__);
    if( device && !uniformBuffers.empty() && !uniformBuffersMemory.empty()) {
        console.log("[Veloxr] [Debug] Destroying uniform data\n");
        for (auto &[name, data] : _textureMap) data.destroy(device);
        console.log("[Veloxr] [Debug] Destroying uniform pools\n");
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    _currentFilepath = "";
    _currentDataBuffer = {};
}

void RendererCore::destroy() {
    console.logc1(__func__);
    if(!device) return;
    console.log("[Veloxr] [Debug] Destroying!", device, "\n");
    cleanupSwapChain();
    destroyTextureData();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
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

void RendererCore::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || frameBufferResized) {
        console.log("[Veloxr]", "Resizing swapchain\n");
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
