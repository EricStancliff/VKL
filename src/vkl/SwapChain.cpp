#include <vkl/SwapChain.h>
#include <vector>
#include <array>

#include <vkl/Device.h>
#include <vkl/Window.h>
#include <vkl/Surface.h>
#include <vkl/SwapChain.h>
#include <vkl/Instance.h>
#include <vkl/RenderPass.h>
#include <vkl/CommandDispatcher.h>

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

        throw std::runtime_error("Error");
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

    SwapChain::SwapChain(const Device& device, const Surface& surface, SwapChainOptions options)
    {
        _options = options;
        init(device, surface);
        createSyncObjects(device, surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsFamilyQueueIndex();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device.handle(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Error");
        }
        _oneOffCommandBuffers.resize(framesInFlight());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)_oneOffCommandBuffers.size();

        if (vkAllocateCommandBuffers(device.handle(), &allocInfo, _oneOffCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error");
        }

        if (!_prepped)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;

            vkBeginCommandBuffer(_oneOffCommandBuffers[_imageIndex], &beginInfo);
            _prepped = true;
        }

    }

    void SwapChain::cleanUp(const Device& device)
    {
        cleanUpSwapChain(device);
        vkFreeCommandBuffers(device.handle(), _commandPool, (uint32_t)_oneOffCommandBuffers.size(), _oneOffCommandBuffers.data());
        vkDestroyCommandPool(device.handle(), _commandPool, nullptr);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device.handle(), _renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device.handle(), _imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device.handle(), _inFlightFences[i], nullptr);
        }
    }

    void SwapChain::cleanUpSwapChain(const Device& device)
    {
        vkDestroyImageView(device.handle(), _depthImageView, nullptr);
        vmaDestroyImage(device.allocatorHandle(), _depthImage, _depthImageMemory);

        vkDestroyImageView(device.handle(), _colorImageView, nullptr);
        vmaDestroyImage(device.allocatorHandle(), _colorImage, _colorImageMemory);

        for (auto framebuffer : _swapChainFramebuffers) {
            vkDestroyFramebuffer(device.handle(), framebuffer, nullptr);
        }

        for (auto imageView : _swapChainImageViews) {
            vkDestroyImageView(device.handle(), imageView, nullptr);
        }

        vkDestroySwapchainKHR(device.handle(), _swapchain, nullptr);
    }

    void SwapChain::reset(const Device& device, const Surface& surface, const CommandDispatcher& commands, const RenderPass& compatiblePass, const WindowSize& targetExtent)
    {
        vkDeviceWaitIdle(device.handle());
        cleanUpSwapChain(device);
        _options.swapChainExtent = targetExtent;
        _imageIndex = 0;
        init(device, surface);
        registerRenderPass(device, compatiblePass);
    }

    void SwapChain::init(const Device& device, const Surface& surface)
    {
        createSwapChain(device, surface);
        createImageViews(device, surface);
        createColorResources(device, surface);
        createDepthResources(device, surface);
    }

    void SwapChain::createSwapChain(const Device& device, const Surface& surface)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device.physicalDeviceHandle(), surface.handle());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, _options);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, _options);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, _options);

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
            throw std::runtime_error("Error");
        }

        vkGetSwapchainImagesKHR(device.handle(), _swapchain, &imageCount, nullptr);
        _swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device.handle(), _swapchain, &imageCount, _swapChainImages.data());

        _swapChainImageFormat = surfaceFormat.format;
        _swapChainExtent = extent;
    }

    void SwapChain::createImageViews(const Device& device, const Surface& surface)
    {
        _swapChainImageViews.resize(_swapChainImages.size());

        for (uint32_t i = 0; i < _swapChainImages.size(); i++) {
            _swapChainImageViews[i] = createImageView(device, _swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    void SwapChain::createColorResources(const Device& device, const Surface& surface)
    {
        VkFormat colorFormat = _swapChainImageFormat;

        createImage(device, _swapChainExtent.width, _swapChainExtent.height, 1, device.maxUsableSamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _colorImage, _colorImageMemory);

        _colorImageView = createImageView(device, _colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    void SwapChain::createDepthResources(const Device& device, const Surface& surface)
    {
        _swapChainDepthFormat = findDepthFormat(device.physicalDeviceHandle());

        createImage(device, _swapChainExtent.width, _swapChainExtent.height, 1, device.maxUsableSamples(), _swapChainDepthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
        
        _depthImageView = createImageView(device, _depthImage, _swapChainDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

    void SwapChain::registerRenderPass(const Device& device, const RenderPass& renderPass)
    {

        _swapChainFramebuffers.resize(_swapChainImageViews.size(), VK_NULL_HANDLE);

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
                throw std::runtime_error("Error");
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
        return _imageIndex;
    }

    VkFramebuffer SwapChain::frameBuffer(size_t frame) const
    {
        return _swapChainFramebuffers[frame];
    }

    VkExtent2D SwapChain::swapChainExtent() const
    {
        return _swapChainExtent;
    }

    VkCommandBuffer SwapChain::oneOffCommandBuffer(size_t frame) const
    {
        return _oneOffCommandBuffers[frame];
    }

    void SwapChain::prepNextFrame(const Device& device, const Surface& surface, const CommandDispatcher& commands, const RenderPass& compatiblePass, const WindowSize& targetExtent)
    {
        vkWaitForFences(device.handle(), 1, &_inFlightFences[_frameClamp], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device.handle(), _swapchain, UINT64_MAX, _imageAvailableSemaphores[_frameClamp], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            reset(device, surface, commands, compatiblePass, targetExtent);
            if (!_prepped)
            {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = 0;

                vkBeginCommandBuffer(_oneOffCommandBuffers[_imageIndex], &beginInfo);
                _prepped = true;
            }
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Error");
        }

        _imageIndex = imageIndex;

        if (!_prepped)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;

            vkBeginCommandBuffer(_oneOffCommandBuffers[_imageIndex], &beginInfo);
            _prepped = true;
        }
    }

    void SwapChain::swap(const Device& device, const Surface& surface, const CommandDispatcher& commands, const RenderPass& compatiblePass, const WindowSize& targetExtent)
    {

        if (_prepped)
        {
            vkEndCommandBuffer(_oneOffCommandBuffers[_imageIndex]);
            _prepped = false;
        }

        bool resized = targetExtent.width != swapChainExtent().width || targetExtent.height != swapChainExtent().height;



        if (_imagesInFlight[_imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device.handle(), 1, &_imagesInFlight[_imageIndex], VK_TRUE, UINT64_MAX);
        }
        _imagesInFlight[_imageIndex] = _inFlightFences[_frameClamp];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_frameClamp] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 2;
        auto cmdBuffer = commands.primaryCommandBuffer(_imageIndex);
        VkCommandBuffer cmdBuffers[2] = { _oneOffCommandBuffers[_imageIndex], cmdBuffer };
        submitInfo.pCommandBuffers = cmdBuffers;

        VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_frameClamp] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkResetFences(device.handle(), 1, &_inFlightFences[_frameClamp]) != VK_SUCCESS) {
            throw std::runtime_error("Error");
        }
        if (vkQueueSubmit(device.graphicsQueueHandle(), 1, &submitInfo, _inFlightFences[_frameClamp]) != VK_SUCCESS) {
            throw std::runtime_error("Error");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        uint32_t imageIndex = (uint32_t)_imageIndex;
        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(device.presentQueueHandle(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
            reset(device, surface, commands, compatiblePass, targetExtent);
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("Error");
        }

        _frameClamp = (_frameClamp + 1) % MAX_FRAMES_IN_FLIGHT;

    }

    void SwapChain::createSyncObjects(const Device& device, const Surface& surface)
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
                throw std::runtime_error("Error");
            }
        }
    }
}