#pragma once
#include <memory>
#include <Common.h>

namespace vkl
{
	class Instance;
	class Surface;

	struct WindowData;

	using WindowHandle = void*;

	class VKL_EXPORT Window
	{
	public:
		Window(int width, int height, const char* title);
		Window() = delete;
		~Window();

		static const char** getRequiredInstanceExtensions(uint32_t* count);

		WindowSize getWindowSize() const;

		//If you need the GLFWwindow*
		WindowHandle handle() const;

	private:
		friend class Surface;
		VkSurfaceKHR createSurfaceHandle_Private(const Instance& instance) const;

		std::unique_ptr<WindowData> _windowData;
	};
}