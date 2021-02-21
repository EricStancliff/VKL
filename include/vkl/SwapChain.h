#pragma once
#include <Common.h>

namespace vkl
{
	struct SwapChainOptions
	{
		//Default to sRGB color space - no need for manual gamma correction
		VkFormat colorFormat{ VK_FORMAT_B8G8R8A8_SRGB };
		VkColorSpaceKHR colorSpace {VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

		//present mode
		VkPresentModeKHR presentMode{ VK_PRESENT_MODE_MAILBOX_KHR };

		//For use with a windowless swapchain
		WindowSize swapChainExtent;
	};

	class VKL_EXPORT SwapChain
	{
	public:
		SwapChain() = delete;
		SwapChain(const Instance& instance, const Device& device, const Surface& surface, const Window& window, SwapChainOptions options = {});
		SwapChain(const Instance& instance, const Device& device, const Surface& surface, SwapChainOptions options = {});
		~SwapChain() = delete;

		//must call after creation before you attempt to render anything
		//don't need all of them, just one "compatible" render pass to make frame buffers
		//https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#renderpass-compatibility
		void registerRenderPass(const Device& device, const RenderPass& renderPass);

		VkFormat imageFormat() const;
		VkFormat depthFormat() const;

		uint32_t graphicsFamilyQueueIndex() const;
		uint32_t presentFamilyQueueIndex() const;

		size_t framesInFlight() const;

	private:
		void init(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options);
		
		void createSwapChain(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options);
		void createImageViews(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options);
		void createColorResources(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options);
		void createDepthResources(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options);
		void createSyncObjects(const Instance& instance, const Device& device, const Surface& surface, const SwapChainOptions& options);

		VkSwapchainKHR _swapchain{ VK_NULL_HANDLE };
		
		std::vector<VkImage> _swapChainImages;
		VkFormat _swapChainImageFormat;
		VkFormat _swapChainDepthFormat;
		VkExtent2D _swapChainExtent;
		std::vector<VkImageView> _swapChainImageViews;
		std::vector<VkFramebuffer> _swapChainFramebuffers;

		VkImage _colorImage;
		VmaAllocation _colorImageMemory;
		VkImageView _colorImageView;

		VkImage _depthImage;
		VmaAllocation _depthImageMemory;
		VkImageView _depthImageView;

		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;
		std::vector<VkFence> _inFlightFences;
		std::vector<VkFence> _imagesInFlight;

		uint32_t _graphicsFamilyQueueIndex{ 0 };
		uint32_t _presentFamilyQueueIndex{ 0 };
	};
}