
#pragma once
#include <vector>
#include <string>
#include <vulkan/vulkan.h>

class GPUTextures {
public:
    GPUTextures(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkCommandPool commandPool,
        VkQueue graphicsQueue
    );

    ~GPUTextures();

    void addTexture(const std::string& filepath);

    void create();

    VkDescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }

    VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }

private:
    struct Texture {
        std::string filepath;
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkCommandPool m_commandPool;
    VkQueue m_graphicsQueue;

    std::vector<Texture> m_textures;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

private:
    void loadTexture(Texture& tex);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage& image, VkDeviceMemory& imageMemory);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

