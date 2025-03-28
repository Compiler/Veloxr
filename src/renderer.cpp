#include "renderer.h"
#include <opencv4/opencv2/opencv.hpp>
void RenderCore::createTextureImage()
{
    // cv::Mat image = cv::imread("C:/Users/ljuek/Downloads/16kmarble.jpeg", cv::IMREAD_UNCHANGED);
    Test t{};
    t.run2(PREFIX + "/Users/ljuek/Downloads/56000.jpg", PREFIX + "/Users/ljuek/Downloads/56000_1.jpg");
    cv::Mat image = cv::imread(PREFIX + "/Users/ljuek/Downloads/16kmarble.jpeg", cv::IMREAD_UNCHANGED);
    if (image.empty())
    {
        throw std::runtime_error("Failed to load texture image with OpenCV!");
    }

    if (image.channels() == 3)
    {
        cv::Mat imageRGBA;
        cv::cvtColor(image, imageRGBA, cv::COLOR_BGR2RGBA);
        image = imageRGBA;
    }
    else if (image.channels() == 1)
    {
        cv::Mat imageRGBA;
        cv::cvtColor(image, imageRGBA, cv::COLOR_GRAY2RGBA);
        image = imageRGBA;
    }

    int texWidth = image.cols;
    int texHeight = image.rows;
    int texChannels = image.channels();

    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) *
                             static_cast<VkDeviceSize>(texHeight) *
                             static_cast<VkDeviceSize>(texChannels);

    std::cout << "Loading texture of size "
              << texWidth << " x " << texHeight << ": "
              << (imageSize / 1024.0 / 1024.0) << " MB" << std::endl;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, image.data, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}