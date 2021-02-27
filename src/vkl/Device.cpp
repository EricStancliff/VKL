#include <Device.h>
#include <vector>
#include <optional>
#include <set>
#include <string>

#include <Instance.h>
#include <Surface.h>

namespace vkl
{

    /**************************Private Impl Helper Functions**********************************/


    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }






    /**************************Device Impl**********************************/
    Device::Device(const Instance& instance, const Surface& surface)
	{
        pickPhysicalDevice(instance, surface);
        createLogicalDevice(instance, surface);

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
        allocatorInfo.physicalDevice = _physicalDevice;
        allocatorInfo.device = _device;
        allocatorInfo.instance = instance.handle();

        vmaCreateAllocator(&allocatorInfo, &_allocator);
    }

    VkDevice Device::handle() const
    {
        return _device;
    }

    VkPhysicalDevice Device::physicalDeviceHandle() const
    {
        return _physicalDevice;
    }

    VkQueue Device::graphicsQueueHandle() const
    {
        return _graphicsQueue;
    }

    VkQueue Device::presentQueueHandle() const
    {
        return _presentQueue;
    }

    VkSampleCountFlagBits Device::maxUsableSamples() const
    {
        return _maxUsableSamples;
    }

    VmaAllocator Device::allocatorHandle() const
    {
        return _allocator;
    }

    void Device::cleanUp()
    {
        vmaDestroyAllocator(_allocator);
        vkDestroyDevice(_device, nullptr);
    }

    void Device::waitIdle()
    {
        vkDeviceWaitIdle(_device);
    }

    void Device::pickPhysicalDevice(const Instance& instance, const Surface& surface)
    {
        //Pick a GPU
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance.handle(), &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Error");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance.handle(), &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device, surface.handle())) {
                _physicalDevice = device;
                break;
            }
        }


        if (_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Error");
        }

        _maxUsableSamples = getMaxUsableSampleCount(_physicalDevice);
    }
    
    void Device::createLogicalDevice(const Instance& instance, const Surface& surface)
    {
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice, surface.handle());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(getVklDeviceExtensions().size());
        createInfo.ppEnabledExtensionNames = getVklDeviceExtensions().data();

        if (!instance.getLayers().empty()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(instance.getLayers().size());
            createInfo.ppEnabledLayerNames = instance.getLayers().data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("Error");
        }

        vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);

    }
}