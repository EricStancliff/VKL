#pragma once
#include <memory>
#include <vkl/Common.h>

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
		Window(const Window&) = delete;
		Window(Window&&) noexcept;
		Window& operator=(Window&&) noexcept;
		Window& operator=(const Window&) = delete;

		static const char** getRequiredInstanceExtensions(uint32_t* count);

		WindowSize getWindowSize() const;

		//If you need the GLFWwindow*
		WindowHandle handle() const;

		bool shouldClose() const;

		static void pollEvents();

		void cleanUp();

		static void cleanUpWindowSystem();

	private:
		friend class Surface;
		VkSurfaceKHR createSurfaceHandle_Private(const Instance& instance) const;

		std::unique_ptr<WindowData> _windowData;
	};
}