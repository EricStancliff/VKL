#pragma once

#include <vkl/Common.h>
#include <memory>

namespace vkl
{
	class CommandThread;
	class PipelineManager;

	class VKL_EXPORT CommandDispatcher
	{
	public:
		~CommandDispatcher();

		CommandDispatcher() = delete;
		CommandDispatcher(const Device& device, const SwapChain& swapChain);
		CommandDispatcher(const CommandDispatcher&) = delete;
		CommandDispatcher(CommandDispatcher&&) noexcept;
		CommandDispatcher& operator=(CommandDispatcher&&) noexcept;
		CommandDispatcher& operator=(const CommandDispatcher&) = delete;

		void processUnsortedObjects(std::span< std::shared_ptr<RenderObject>> objects, const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const VkExtent2D& extent);

		VkCommandBuffer primaryCommandBuffer(size_t frame) const;

		void cleanUp(const Device& device);
	private:
		VkCommandPool _commandPool{ VK_NULL_HANDLE };
		std::vector<std::unique_ptr<CommandThread>> _threads;
		std::vector<VkCommandBuffer> _primaryBuffers;
	};
}