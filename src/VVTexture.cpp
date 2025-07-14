#include "VVTexture.h"
#include "DataUtils.h"
#include "TextureTiling.h"
#include "VVUtils.h"


namespace Veloxr {

VVTexture::VVTexture(std::shared_ptr<VVDataPacket> dataPacket): _data(dataPacket) {}

void VVTexture::setDataPacket(std::shared_ptr<VVDataPacket> dataPacket) {
    _data = dataPacket;
}

void VVTexture::tileTexture(std::shared_ptr<Veloxr::VeloxrBuffer> buffer) {
    console.logc2(__func__, buffer->data.size());
    auto now = std::chrono::high_resolution_clock::now();
    static Veloxr::TextureTiling tiler{};

    // TODO: Calculate best time case for maxResolution. 4096 will be 200% faster that maxresolution, but not sure if they will have enough sampelrs
    // TODO2: Use indexed binding on hardware that supports it.
    Veloxr::TiledResult tileData = tiler.tile(buffer, 8192);

    auto timeToTileMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count();
    now = std::chrono::high_resolution_clock::now();
    for(const auto& [indx, tileData] : tileData.tiles){

        int texWidth    = tileData.width;
        int texHeight   = tileData.height;
        int texChannels = 4;//myTexture.getNumChannels();
        int samplerIndex = tileData.samplerIndex;
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * 
            static_cast<VkDeviceSize>(texHeight) *
            static_cast<VkDeviceSize>(texChannels);

        console.log("Loading texture of size ", texWidth, " x ", texHeight, ": ", (imageSize / 1024.0 / 1024.0), " MB");

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VVUtils::createBuffer(_data, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(_data->device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, tileData.pixelData.data(), static_cast<size_t>(imageSize));
        vkUnmapMemory(_data->device, stagingBufferMemory);

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        // not thread safe cuz of command pool
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // thread safe
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(_data->device, stagingBuffer, nullptr);
        vkFreeMemory(_data->device, stagingBufferMemory, nullptr);
        textureImage = textureImage;
        textureImageMemory = textureImageMemory;

        
        auto imageView = createTextureImageView(textureImage);
        auto sampler = createTextureSampler();
        textureImageView = imageView;
        textureSampler = sampler;
        samplerIndex = samplerIndex;
    }
    auto timeToUploadMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count();

    _vertices.insert(_vertices.begin(), std::make_move_iterator(tileData.vertices.begin()), std::make_move_iterator(tileData.vertices.end()));

    float minX = +9999.0f, maxX = -9999.0f;
    float minY = +9999.0f, maxY = -9999.0f;
    for (auto &v : _vertices) {
        minX = std::min(minX, v.pos.x);
        maxX = std::max(maxX, v.pos.x);
        minY = std::min(minY, v.pos.y);
        maxY = std::max(maxY, v.pos.y);
    }
    console.log("Final geometry bounding box: X in [", minX, ", ", maxX, "], Y in [", minY, ", ", maxY, "]");

    console.fatal("Time to tile: ", timeToTileMs, " ms");
    console.fatal("Time to upload data: ", timeToUploadMs, " ms");
}

void VVTexture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    //console.logc1(__func__);
    VkCommandBuffer commandBuffer = CommandUtils::beginSingleTimeCommands(_data->device, _data->commandPool);

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
    CommandUtils::endSingleTimeCommands(_data->device, commandBuffer, _data->commandPool, _data->graphicsQueue);
}

void VVTexture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    //console.logc1(__func__);
    VkCommandBuffer commandBuffer = CommandUtils::beginSingleTimeCommands(_data->device, _data->commandPool);

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

    CommandUtils::endSingleTimeCommands(_data->device, commandBuffer, _data->commandPool, _data->graphicsQueue);
}

void VVTexture::createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory) {
    //console.logc1(__func__);
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

    if (vkCreateImage(_data->device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_data->device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VVUtils::findMemoryType(_data, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_data->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_data->device, image, imageMemory, 0);
}

void VVTexture::destroy() {
    if(!_data->device) {
        console.warn("Called destroy on VVTexture with no device.");
        return;
    }

    console.log("Destroying on device: ", _data->device);

    if (textureSampler) {
        console.logc1("Destroying Sampler");
        vkDestroySampler(_data->device, textureSampler, nullptr);
        console.logc1("Destroyed.");
    }

    if ( textureImageView ) {
        console.logc1("Destroying ImageView");
        vkDestroyImageView(_data->device, textureImageView, nullptr);
        console.logc1("Destroyed.");
    }

    if ( textureImage ) {
        console.logc1("Destroying Image");
        vkDestroyImage(_data->device, textureImage, nullptr);
        console.logc1("Destroyed.");
    }

    if ( textureImageMemory ) {
        console.logc1("Freeing textureImageMemory");
        vkFreeMemory(_data->device, textureImageMemory, nullptr);
        console.logc1("Destroyed.");
    }
}

VkImageView VVTexture::createImageView(VkImage image, VkFormat format) {
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
    if (vkCreateImageView(_data->device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}
VkImageView VVTexture::createTextureImageView(VkImage textureImage) {
    console.logc1(__func__);
    return createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

VkSampler VVTexture::createTextureSampler() {
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

    if (vkCreateSampler(_data->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    return textureSampler;
}

VVTexture::~VVTexture() {
    this->destroy();
}

}
