#pragma once

#include <Common.h>
#include <memory>
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

	private:
		std::vector<std::unique_ptr<CommandThread>> _threads;
	
	};
}