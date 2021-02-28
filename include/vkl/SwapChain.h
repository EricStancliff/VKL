#pragma once
#include <vkl/Common.h>

namespace vkl
{
	struct SwapChainOptions
	{
		//Default to sRGB color space - no need for manual gamma correction
		VkFormat colorFormat{ VK_FORMAT_B8G8R8A8_SRGB };
		VkColorSpaceKHR colorSpace {VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

		//present mode
		VkPresentModeKHR presentMode{ VK_PRESENT_MODE_MAILBOX_KHR };

		WindowSize swapChainExtent;
	};

	class VKL_EXPORT SwapChain
	{
	public:
		SwapChain() = delete;
		~SwapChain() = default;
		SwapChain(const SwapChain&) = delete;
		SwapChain(SwapChain&&) noexcept = default;
		SwapChain& operator=(SwapChain&&) noexcept = default;
		SwapChain& operator=(const SwapChain&) = delete;

		SwapChain(const Device& device, const Surface& surface, SwapChainOptions options = {});

		void cleanUp(const Device& device);

		//must call after creation before you attempt to render anything
		//don't need all of them, just one "compatible" render pass to make frame buffers
		//https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#renderpass-compatibility
		void registerRenderPass(const Device& device, const RenderPass& renderPass);

		VkFormat imageFormat() const;
		VkFormat depthFormat() const;

		uint32_t graphicsFamilyQueueIndex() const;
		uint32_t presentFamilyQueueIndex() const;

		size_t framesInFlight() const;
		size_t frame() const;

		VkFramebuffer frameBuffer(size_t frame) const;

		VkExtent2D swapChainExtent() const;

		VkCommandBuffer oneOffCommandBuffer(size_t frame) const;

		void prepNextFrame(const Device& device, const Surface& surface, const CommandDispatcher& commands, const RenderPass& compatiblePass, const WindowSize& targetExtent);
		void swap(const Device& device, const Surface& surface, const CommandDispatcher& commands, const RenderPass& compatiblePass, const WindowSize& targetExtent);

	private:
		void cleanUpSwapChain(const Device& device);

		void reset(const Device& device, const Surface& surface, const CommandDispatcher& commands, const RenderPass& compatiblePass, const WindowSize& targetExtent);

		void init(const Device& device, const Surface& surface);
		
		void createSwapChain(const Device& device, const Surface& surface);
		void createImageViews(const Device& device, const Surface& surface);
		void createColorResources(const Device& device, const Surface& surface);
		void createDepthResources(const Device& device, const Surface& surface);
		void createSyncObjects(const Device& device, const Surface& surface);

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

		size_t _imageIndex{ 0 };
		size_t _frameClamp{ 0 };

		SwapChainOptions _options;

		std::vector<VkCommandBuffer> _oneOffCommandBuffers;
		VkCommandPool _commandPool{ VK_NULL_HANDLE };
		bool _prepped{ false };
	};
}