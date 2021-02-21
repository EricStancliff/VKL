#pragma once
#include <Common.h>

namespace vkl
{
	struct RenderPassOptions
	{
		//TODO - expand to handle different formats and subpass dependencies
	};


	class VKL_EXPORT RenderPass
	{
	public:
		RenderPass() = delete;
		//passing in a swapchain enforces swapchain compatibility
		RenderPass(const Device& device, const SwapChain& swapChain, const RenderPassOptions& options = {});
		~RenderPass() = default;


		VkRenderPass handle() const;

	private:
		VkRenderPass _renderPass{ VK_NULL_HANDLE };
	};
}