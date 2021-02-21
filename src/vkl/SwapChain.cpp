#include <SwapChain.h>
#include <vector>
#include <array>

#include <Device.h>
#include <Window.h>
#include <Surface.h>
#include <SwapChain.h>
#include <Instance.h>
#include <RenderPass.h>


namespace vkl
{
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, const SwapChainOptions& options) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == options.colorFormat && availableFormat.colorSpace == options.colorSpace) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const SwapChainOptions& options) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == options.presentMode) {
                return availablePresentMode;
            }
        }

        //TODO - LOG
        return VK_PRESENT_MODE_FIFO_KHR; //fallback who's impl is required for compliance
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const SwapChainOptions& options) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        else {

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(options.swapChainExtent.width),
                static_cast<uint32_t>(options.swapChainExtent.height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        //TODO - LOG
        return VK_FORMAT_UNDEFINED;
    }

    VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
        return findSupportedFormat(physicalDevice,
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    /******************************************************************************************************************/


	SwapChain::SwapChain(const Instance& instance, const Device& device, const Surface& surface, const Window& window, SwapChainOptions options)
	{
        options.swapChainExtent = window.getWindowSize();
        init(instance, device, surface, options);
	}

    SwapChain::SwapChain(const Instance& instance, const Device& device, const Surface& surface, SwapChainOptions options)
    {
        init(instance, device, surface, options);
    }

    void SwapChain::init(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options)
    {
        createSwapChain(instance, device, surface, options);
        createImageViews(instance, device, surface, options);
        createColorResources(instance, device, surface, options);
        createDepthResources(instance, device, surface, options);
        createSyncObjects(instance, device, surface, options);
    }

    void SwapChain::createSwapChain(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device.physicalDeviceHandle(), surface.handle());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, options);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, options);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, options);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface.handle();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(device.physicalDeviceHandle(), surface.handle());
        
        _graphicsFamilyQueueIndex = indices.graphicsFamily.value();
        _presentFamilyQueueIndex = indices.presentFamily.value();

        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device.handle(), &createInfo, nullptr, &_swapchain) != VK_SUCCESS) {
            //TODO - LOG
        }

        vkGetSwapchainImagesKHR(device.handle(), _swapchain, &imageCount, nullptr);
        _swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device.handle(), _swapchain, &imageCount, _swapChainImages.data());

        _swapChainImageFormat = surfaceFormat.format;
        _swapChainExtent = extent;
    }

    void SwapChain::createImageViews(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options)
    {
        _swapChainImageViews.resize(_swapChainImages.size());

        for (uint32_t i = 0; i < _swapChainImages.size(); i++) {
            _swapChainImageViews[i] = createImageView(device, _swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    void SwapChain::createColorResources(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options)
    {
        VkFormat colorFormat = _swapChainImageFormat;

        createImage(device, _swapChainExtent.width, _swapChainExtent.height, 1, device.maxUsableSamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _colorImage, _colorImageMemory);

        _colorImageView = createImageView(device, _colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    void SwapChain::createDepthResources(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options)
    {
        _swapChainDepthFormat = findDepthFormat(device.physicalDeviceHandle());

        createImage(device, _swapChainExtent.width, _swapChainExtent.height, 1, device.maxUsableSamples(), _swapChainDepthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
        
        _depthImageView = createImageView(device, _depthImage, _swapChainDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

    void SwapChain::registerRenderPass(const Device& device, const RenderPass& renderPass)
    {

        _swapChainFramebuffers.resize(_swapChainImageViews.size());

        for (size_t i = 0; i < _swapChainImageViews.size(); i++) {
            std::array<VkImageView, 3> attachments = {
                _colorImageView,
                _depthImageView,
                _swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass.handle();// need "compatible" render pass
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = _swapChainExtent.width;
            framebufferInfo.height = _swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.handle(), &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
                //TODO - LOG
            }
        }
    }

    VkFormat SwapChain::imageFormat() const
    {
        return _swapChainImageFormat;
    }

    VkFormat SwapChain::depthFormat() const
    {
        return _swapChainDepthFormat;
    }

    uint32_t SwapChain::graphicsFamilyQueueIndex() const
    {
        return _graphicsFamilyQueueIndex;
    }

    uint32_t SwapChain::presentFamilyQueueIndex() const
    {
        return _presentFamilyQueueIndex;
    }

    size_t SwapChain::framesInFlight() const
    {
        return _swapChainImageViews.size();
    }

    size_t SwapChain::frame() const
    {
        return _frame;
    }

    void SwapChain::createSyncObjects(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options)
    {
        _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        _imagesInFlight.resize(_swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device.handle(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device.handle(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device.handle(), &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
                //TODO - LOG
            }
        }
    }
}