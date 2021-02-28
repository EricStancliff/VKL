#pragma once
#include <vkl/Common.h>

namespace vkl
{
	struct RenderPassOptions
	{
		//TODO - expand to handle different formats and subpass dependencies
		VkClearColorValue clearColor = { 0.f, 0.f, 0.f, 1.f };
		VkClearDepthStencilValue clearDepthStencil = { 1.f, 0 };
	};


	class VKL_EXPORT RenderPass
	{
	public:
		RenderPass() = delete;
		//passing in a swapchain enforces swapchain compatibility
		RenderPass(const Device& device, const SwapChain& swapChain, const RenderPassOptions& options = {});
		~RenderPass() = default;


		VkRenderPass handle() const;

		const RenderPassOptions& options() const;

		void cleanUp(const Device& device);

	private:
		VkRenderPass _renderPass{ VK_NULL_HANDLE };
		RenderPassOptions _options;
	};
}