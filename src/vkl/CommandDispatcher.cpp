#include <vkl/CommandDispatcher.h>

#include <vkl/Device.h>
#include <vkl/SwapChain.h>
#include <vkl/RenderObject.h>
#include <vkl/RenderPass.h>

#include <future>
#include <iostream>
#include <array>

#define MULTITHREADED
#define TBB_THREADING

#ifdef TBB_THREADING
#include <tbb/parallel_for.h>
#endif

namespace vkl
{
	class CommandThread
	{
	public:


		CommandThread(const Device& device, const SwapChain& swapChain)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = swapChain.graphicsFamilyQueueIndex();
			poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			if (vkCreateCommandPool(device.handle(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
				throw std::runtime_error("Error");
			}

			_commandBuffers.resize(swapChain.framesInFlight());

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = _commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

			if (vkAllocateCommandBuffers(device.handle(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
				throw std::runtime_error("Error");
			}

		}
		CommandThread() = delete;
		~CommandThread() = default;
		CommandThread(const CommandThread&) = delete;
		CommandThread(CommandThread&&) noexcept = default;
		CommandThread& operator=(CommandThread&&) noexcept = default;
		CommandThread& operator=(const CommandThread&) = delete;

		void startThread()
		{
			_running = true;
			_thread = std::make_unique<std::thread>([this] {run(); });
		}

		void begin(const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const VkExtent2D& extent)
		{
			VkCommandBufferInheritanceInfo inherit{};
			inherit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inherit.framebuffer = frameBuffer;
			inherit.renderPass = pass.handle();
			inherit.subpass = 0;//todo

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = &inherit;

			if (vkBeginCommandBuffer(_commandBuffers[swapChain.frame()], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Error");
			}

			VkViewport viewport{};
			viewport.height = (float)extent.height;
			viewport.width = (float)extent.width;
			viewport.minDepth = 0.0;
			viewport.maxDepth = 1.0;
			vkCmdSetViewport(_commandBuffers[swapChain.frame()], 0, 1, &viewport);

		}

		VkCommandBuffer end(const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const VkExtent2D& extent)
		{
			if (vkEndCommandBuffer(_commandBuffers[swapChain.frame()]) != VK_SUCCESS) {
				throw std::runtime_error("Error");
			}

			return _commandBuffers[swapChain.frame()];
		}
		void processObjectsNow(std::span<const std::shared_ptr<RenderObject>> objects, const Device& device, const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const VkExtent2D& extent)
		{
			for (auto&& object : objects)
			{
				object->updateDescriptors(device, swapChain, pipelines);
				object->recordCommands(swapChain, pipelines, _commandBuffers[swapChain.frame()], extent);
			}
		}
		void processObjectNow(RenderObject& object, const PipelineManager& pipelines, const Device& device, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const VkExtent2D& extent)
		{
			object.updateDescriptors(device, swapChain, pipelines);
			object.recordCommands(swapChain, pipelines, _commandBuffers[swapChain.frame()], extent);
		}

		std::future<bool> processObjects(std::span<const std::shared_ptr<RenderObject>> objects, const Device& device, const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const VkExtent2D& extent)
		{
			std::future<bool> future;
			auto promise = std::make_shared<std::promise<bool>>();
			future = promise->get_future();
			{
				std::unique_lock<std::mutex> lock(_mutex);
				_tasks.emplace_back([objects, &pipelines, &pass, &swapChain, frameBuffer, &extent, &device, promise, this]() {
					processObjectsNow(objects, device, pipelines, pass, swapChain, frameBuffer, extent);
					promise->set_value(true);
					});
			}
			_condition.notify_all();
			return future;
		}

		VkCommandBuffer handle(size_t frame) const
		{
			return _commandBuffers[frame];
		}
		void cleanUp(const Device& device)
		{
			bool killThread = false;
			{
				std::unique_lock<std::mutex> lock(_mutex);
				killThread = _running;
				_running = false;
			}
			if (killThread)
			{
				_condition.notify_all();

				_thread->join();
				_thread = nullptr;
			}

			vkFreeCommandBuffers(device.handle(), _commandPool, (uint32_t)_commandBuffers.size(), _commandBuffers.data());
			vkDestroyCommandPool(device.handle(), _commandPool, nullptr);
		}
	private:

		void run()
		{
			while (true)
			{
				std::vector < std::function<void()>> localTasks;
				bool quit = false;
				{
					std::unique_lock<std::mutex> lock(_mutex);
					_condition.wait(lock, [this]() {
						return !_tasks.empty() || !_running;
						});
					localTasks = std::move(_tasks);
					_tasks.clear();
					quit = !_running;
				}
				//should we finish tasks before bailing?
				for (auto&& task : localTasks)
				{
					task();
				}
				if (quit)
					break;
			}
		}
		VkCommandPool _commandPool{ VK_NULL_HANDLE };
		std::vector<VkCommandBuffer> _commandBuffers;

		std::unique_ptr<std::thread> _thread;
		bool _running = false;
		std::mutex _mutex;
		std::condition_variable _condition;
		std::vector < std::function<void()>> _tasks;
		
	};


	CommandDispatcher::CommandDispatcher(const Device& device, const SwapChain& swapChain)
	{
#ifdef MULTITHREADED
		unsigned int threadCount = std::thread::hardware_concurrency();
#else
		unsigned int threadCount = 1;
#endif
		for (unsigned int i = 0; i < threadCount; ++i)
		{
			_threads.emplace_back(std::move(std::make_unique<CommandThread>(device, swapChain)));
#ifdef MULTITHREADED
#ifndef TBB_THREADING
			_threads.back()->startThread();
#endif
#endif
		}


		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = swapChain.graphicsFamilyQueueIndex();   
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device.handle(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}
		_primaryBuffers.resize(swapChain.framesInFlight());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)_primaryBuffers.size();

		if (vkAllocateCommandBuffers(device.handle(), &allocInfo, _primaryBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}

	}

	CommandDispatcher::CommandDispatcher(CommandDispatcher&&) noexcept = default;
	CommandDispatcher& CommandDispatcher::operator=(CommandDispatcher&&) noexcept = default;

	CommandDispatcher::~CommandDispatcher()
	{
	}
	void CommandDispatcher::processUnsortedObjects(std::span< std::shared_ptr<RenderObject>> objects, const Device& device, const PipelineManager& pipelines, const RenderPass& pass, const SwapChain& swapChain, VkFramebuffer frameBuffer, const VkExtent2D& extent)
	{

#ifdef MULTITHREADED
		size_t objectCount = objects.size();
		size_t objectsPerThread = objectCount / _threads.size() + 1;

		std::vector<VkCommandBuffer> buffers;
#ifdef TBB_THREADING
		tbb::parallel_for(tbb::blocked_range<size_t>(0, _threads.size()), [&](const tbb::blocked_range<size_t>& r)
			{
				for (size_t i = r.begin(); i != r.end(); ++i)
				{
					size_t offset = (i * objectsPerThread);
					auto objsToDo = std::min(objectsPerThread, objects.size() - offset);
					_threads[i]->begin(pipelines, pass, swapChain, frameBuffer, extent);
					_threads[i]->processObjectsNow(objects.subspan(offset, objsToDo), device, pipelines, pass, swapChain, frameBuffer, extent);
					VkCommandBuffer buffer = _threads[i]->end(pipelines, pass, swapChain, frameBuffer, extent);
					buffers.push_back(buffer);
				}
	});
#else
		std::vector<std::future<bool>> futures;
		for (size_t i = 0; i < _threads.size(); ++i)
		{
			size_t offset = (i * objectsPerThread);
			auto objsToDo = std::min(objectsPerThread, objects.size() - offset);
			_threads[i]->begin(pipelines, pass, swapChain, frameBuffer, extent);
			futures.push_back(_threads[i]->processObjects(objects.subspan(offset, objsToDo), device, pipelines, pass, swapChain, frameBuffer, extent));
		}
		for (auto&& future : futures)
			future.get();
		for (auto&& thread : _threads)
			buffers.push_back(thread->end(pipelines, pass, swapChain, frameBuffer, extent));

#endif
#else
		for (auto&& ro : objects)
			ro->updateDescriptors(device, swapChain, pipelines);
		_threads[0]->begin(pipelines, pass, swapChain, frameBuffer, extent);
		_threads[0]->processObjectsNow(objects, device, pipelines, pass, swapChain, frameBuffer, extent);
		std::vector<VkCommandBuffer> buffers;
		VkCommandBuffer buffer = _threads[0]->end(pipelines, pass, swapChain, frameBuffer, extent);
		buffers.push_back(buffer);
#endif

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;

		if (vkBeginCommandBuffer(_primaryBuffers[swapChain.frame()], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pass.handle();
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = pass.options().clearColor;
		clearValues[1].depthStencil = pass.options().clearDepthStencil;

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_primaryBuffers[swapChain.frame()], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		vkCmdExecuteCommands(_primaryBuffers[swapChain.frame()], (uint32_t)buffers.size(), buffers.data());

		vkCmdEndRenderPass(_primaryBuffers[swapChain.frame()]);

		if (vkEndCommandBuffer(_primaryBuffers[swapChain.frame()]) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}

	}
	VkCommandBuffer CommandDispatcher::primaryCommandBuffer(size_t frame) const
	{
		return _primaryBuffers[frame];
	}
	void CommandDispatcher::cleanUp(const Device& device)
	{
		for (auto&& thread : _threads)
			thread->cleanUp(device);
		vkFreeCommandBuffers(device.handle(), _commandPool, (uint32_t)_primaryBuffers.size(), _primaryBuffers.data());
		vkDestroyCommandPool(device.handle(), _commandPool, nullptr);
	}
}