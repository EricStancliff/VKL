#pragma once

#include <Common.h>
#include <memory>
#include <Pipeline.h>

namespace vkl
{
	class CommandThread;
	class VKL_EXPORT CommandDispatcher
	{
	public:

		CommandDispatcher(const Device& device, const SwapChain& swapChain);
		~CommandDispatcher();
		CommandDispatcher(const CommandDispatcher&) = delete;
		CommandDispatcher(CommandDispatcher&&) noexcept = default;
		CommandDispatcher& operator=(CommandDispatcher&&) noexcept = default;
		CommandDispatcher& operator=(const CommandDispatcher&) = delete;

		void processUnsortedObjects(std::span<RenderObject> objects, const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const WindowSize& extent);

	private:
		std::vector<std::unique_ptr<CommandThread>> _threads;
		std::vector<VkCommandBuffer> _primaryBuffers;
	};
}