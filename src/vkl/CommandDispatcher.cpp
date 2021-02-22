#include <CommandDispatcher.h>

#include <Device.h>
#include <SwapChain.h>
#include <RenderObject.h>
#include <RenderPass.h>

#include <future>
#include <iostream>
#include <array>

namespace vkl
{
	class CommandThread
	{
	public:
		template <class F>
		auto work(F&& f)
		{
			return std::async(std::forward<F>(f));
		}

		CommandThread(const Device& device, const SwapChain& swapChain)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = swapChain.graphicsFamilyQueueIndex();

			if (vkCreateCommandPool(device.handle(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
				//TODO - LOG
			}

			_commandBuffers.resize(swapChain.framesInFlight());

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = _commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

			if (vkAllocateCommandBuffers(device.handle(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
				//TODO - LOG
			}

		}
		CommandThread() = delete;
		~CommandThread() = default;
		CommandThread(const CommandThread&) = delete;
		CommandThread(CommandThread&&) noexcept = default;
		CommandThread& operator=(CommandThread&&) noexcept = default;
		CommandThread& operator=(const CommandThread&) = delete;

		std::future<void> processObjects(std::span<RenderObject> objects, const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const WindowSize& extent)
		{
			return std::async( [&]() {

				VkCommandBufferInheritanceInfo inherit{};
				inherit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inherit.framebuffer = frameBuffer;
				inherit.renderPass = pass.handle();
				inherit.subpass = 0;//todo
				

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.pNext = &inherit;

				if (vkBeginCommandBuffer(_commandBuffers[swapChain.frame()], &beginInfo) != VK_SUCCESS) {
					//TODO - LOG
				}

				for (auto&& object : objects)
				{
					object.recordCommands(swapChain, pipelines, _commandBuffers[swapChain.frame()]);
				}

				if (vkEndCommandBuffer(_commandBuffers[swapChain.frame()]) != VK_SUCCESS) {
					//TODO - LOG
				}

			});
		}

	private:
		VkCommandPool _commandPool{ VK_NULL_HANDLE };
		std::vector<VkCommandBuffer> _commandBuffers;

		std::thread _thread;

		
	};


	CommandDispatcher::CommandDispatcher(const Device& device, const SwapChain& swapChain)
	{
		unsigned int threadCount = std::thread::hardware_concurrency();
		
		for (unsigned int i = 0; i < threadCount; ++i)
			_threads.emplace_back(std::move(std::make_unique<CommandThread>(device, swapChain)));
	}
	CommandDispatcher::~CommandDispatcher()
	{
	}
	void CommandDispatcher::processUnsortedObjects(std::span<RenderObject> objects, const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const WindowSize& extent)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(_primaryBuffers[swapChain.frame()], &beginInfo) != VK_SUCCESS) {
			//TODO - LOG
		}

		VkExtent2D vkExtent{};
		vkExtent.width = extent.width;
		vkExtent.height = extent.height;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pass.handle();
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vkExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = pass.options().clearColor;
		clearValues[1].depthStencil = pass.options().clearDepthStencil;

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_primaryBuffers[swapChain.frame()], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		//record commands
		size_t objectCount = objects.size();
		size_t objectsPerThread = objectCount / _threads.size() + 1;
		
		size_t offset = 0;
		size_t threadIndex = 0;

		std::vector<std::future<void>> futures;

		while (offset < objects.size())
		{
			futures.push_back(_threads[threadIndex]->processObjects(objects.subspan(offset, objectsPerThread),pipelines, pass, swapChain, frameBuffer, extent));
			offset += objectsPerThread;
			++threadIndex;
		}
		for (auto&& future : futures)
			future.wait();

		vkCmdEndRenderPass(_primaryBuffers[swapChain.frame()]);

		if (vkEndCommandBuffer(_primaryBuffers[swapChain.frame()]) != VK_SUCCESS) {
			//TODO - LOG
		}

	}
}