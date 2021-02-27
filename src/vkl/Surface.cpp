#include "Surface.h"

namespace vkl
{
	Surface::Surface(const Instance& instance, const Window& window)
	{
		_handle = window.createSurfaceHandle_Private(instance);
	}
	VkSurfaceKHR Surface::handle() const
	{
		return _handle;
	}
	void Surface::cleanUp(const Instance& instance)
	{
		vkDestroySurfaceKHR(instance.handle(), _handle, nullptr);
	}
}