#include <Window.h>

#include <GLFW/glfw3.h>

#include <Instance.h>

namespace
{
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto userWindow = reinterpret_cast<vkl::Window*>(glfwGetWindowUserPointer(window));
	}
}

namespace vkl
{
	bool isGLFWInitialized()
	{
		static bool localIsInitialized = false;
		if (!localIsInitialized)
		{
			localIsInitialized = glfwInit() == GLFW_TRUE;
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		return localIsInitialized;
	}
	bool s_initialized = isGLFWInitialized();

	struct WindowData
	{
		GLFWwindow* window{ nullptr };
	};

	Window::Window(int width, int height, const char* title)
	{
		if (!isGLFWInitialized())
			return;

		_windowData = std::make_unique<WindowData>();


		_windowData->window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwSetWindowUserPointer(_windowData->window, this);
	}
	Window::~Window()
	{
	}
	const char** Window::getRequiredInstanceExtensions(uint32_t* count)
	{
		return glfwGetRequiredInstanceExtensions(count);
	}

	WindowSize Window::getWindowSize() const
	{
		int width, height;
		glfwGetFramebufferSize(_windowData->window, &width, &height);
		return { (uint32_t)width, (uint32_t)height };
	}

	WindowHandle Window::handle() const
	{
		return (void*)_windowData->window;
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(_windowData->window);
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

	VkSurfaceKHR Window::createSurfaceHandle_Private(const Instance& instance) const
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(instance.handle(), _windowData->window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Error");
		}
		return surface;
	}
}