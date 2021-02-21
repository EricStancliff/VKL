#include <CommandDispatcher.h>

#include <Device.h>
#include <SwapChain.h>

#include <future>
#include <iostream>
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
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
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
}