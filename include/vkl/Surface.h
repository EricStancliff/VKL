#pragma once
#include <vkl/Common.h>
#include <vkl/Window.h>
#include <vkl/Instance.h>

namespace vkl
{
	class VKL_EXPORT Surface
	{
	public:
		Surface() = delete;
		Surface(const Instance& instance, const Window& window);
		~Surface() = default;

		VkSurfaceKHR handle() const;

		void cleanUp(const Instance& instance);

	private:
		VkSurfaceKHR _handle;
	};
}