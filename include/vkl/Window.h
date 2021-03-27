#pragma once
#include <memory>
#include <vkl/Common.h>

namespace vkl
{
	class Instance;
	class Surface;
	class Event;

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

		//poll once for all windows, then update each one
		void clearLastFrame();
		static void pollEventsForAllWindows();
		void updateToThisFrame();

		std::span<std::unique_ptr<const Event>> events() const;

		void cleanUp();

		static void cleanUpWindowSystem();

		bool isFocused() const;
		int width() const;
		int height() const;
		int x() const;
		int y() const;


	private:
		friend class Surface;
		VkSurfaceKHR createSurfaceHandle_Private(const Instance& instance) const;

		std::unique_ptr<WindowData> _windowData;
	};
}