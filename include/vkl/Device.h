#pragma once
#include <Common.h>


namespace vkl
{
	class VKL_EXPORT Device
	{
	public:
		Device() = delete;
		Device(const Instance& instance, const Surface& surface);
		~Device() = default;

		VkDevice handle() const;
		VkPhysicalDevice physicalDeviceHandle() const;
		VkQueue graphicsQueueHandle() const;
		VkQueue presentQueueHandle() const;
		VkSampleCountFlagBits maxUsableSamples() const;

		//We use VMA for vulkan memory - don't allocate your own buffers/images
		VmaAllocator allocatorHandle() const;
	private:

		void pickPhysicalDevice(const Instance& instance, const Surface& surface);
		void createLogicalDevice(const Instance& instance, const Surface& surface);

		VkSampleCountFlagBits _maxUsableSamples{ VK_SAMPLE_COUNT_1_BIT };

		VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };

		VkDevice _device{ VK_NULL_HANDLE };

		VkQueue _graphicsQueue{ VK_NULL_HANDLE };
		VkQueue _presentQueue{ VK_NULL_HANDLE };

		VmaAllocator _allocator;

	};

}