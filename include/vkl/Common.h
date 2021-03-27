#pragma once
#include <vulkan/vulkan.h>

#include <vkl/VKL_EXPORT.h>

#include <vk_mem_alloc.h>

#include <vector>
#include <optional>
#include <span>
#include <stdexcept>


#define VKL_VULKAN_VERSION VK_MAKE_VERSION(1, 0, 0)
#define VKL_ENGINE_NAME "VKL"
#define MAX_FRAMES_IN_FLIGHT 2

namespace vkl
{
    class Device;
    class Instance;
    class SwapChain;
    class Window;
    class Surface;
    class RenderPass;
    class Pipeline;
    class PipelineDescription;
    class UniformBuffer;
    class VertexBuffer;
    class TextureBuffer;
    class DrawCall;
    class IndexBuffer;
    class RenderObject;
    class CommandDispatcher;
    class BufferManager;
    class PushConstantBase;

    struct WindowSize
    {
        uint32_t width = 0;
        uint32_t height = 0;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VKL_EXPORT QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    VKL_EXPORT bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    VKL_EXPORT SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

    VKL_EXPORT VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

    VKL_EXPORT std::span<const char* const> getVklDeviceExtensions();

    VKL_EXPORT VkImageView createImageView(const Device& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    VKL_EXPORT void createImage(const Device& device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& imageMemory);

    VKL_EXPORT 	void transitionImageLayout(const Device& device, const SwapChain& swapChain, size_t frame, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    VKL_EXPORT void copyBufferToImage(const Device& device, const SwapChain& swapChain, size_t frame, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VKL_EXPORT void generateMipmaps(const Device& device, const SwapChain& swapChain, size_t frame, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
}