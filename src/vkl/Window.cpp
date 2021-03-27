#include <vkl/Window.h>
#include <vkl/Instance.h>
#include <vkl/Event.h>

#define USE_GLFW

#ifdef USE_GLFW
#include <GLFW/glfw3.h>

namespace vkl
{
	struct WindowData
	{
		GLFWwindow* window{ nullptr };
		std::vector<std::unique_ptr<const Event>> events;
		int x{ 0 }, y{ 0 }, width{ 0 }, height{ 0 };
		bool focused{ false };
	};
}


namespace
{
	vkl::MouseButton glfwMouseButtonToVkl(int button)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			return vkl::MouseButton::LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return vkl::MouseButton::RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return vkl::MouseButton::MIDDLE;
		}
		return vkl::MouseButton::LEFT;
	}
	vkl::Key glfwKeyToVkl(int key)
	{
		switch (key)
		{
		case GLFW_KEY_SPACE:
			return vkl::Key::KEY_SPACE;
		case GLFW_KEY_APOSTROPHE:
			return vkl::Key::KEY_APOSTROPHE;
		case GLFW_KEY_COMMA:
			return vkl::Key::KEY_COMMA;
		case GLFW_KEY_MINUS:
			return vkl::Key::KEY_MINUS;
		case GLFW_KEY_PERIOD:
			return vkl::Key::KEY_PERIOD;
		case GLFW_KEY_SLASH:
			return vkl::Key::KEY_SLASH;
		case GLFW_KEY_0:
			return vkl::Key::KEY_0;
		case GLFW_KEY_1:
			return vkl::Key::KEY_1;
		case GLFW_KEY_2:
			return vkl::Key::KEY_2;
		case GLFW_KEY_3:
			return vkl::Key::KEY_3;
		case GLFW_KEY_4:
			return vkl::Key::KEY_4;
		case GLFW_KEY_5:
			return vkl::Key::KEY_5;
		case GLFW_KEY_6:
			return vkl::Key::KEY_6;
		case GLFW_KEY_7:
			return vkl::Key::KEY_7;
		case GLFW_KEY_8:
			return vkl::Key::KEY_8;
		case GLFW_KEY_9:
			return vkl::Key::KEY_9;
		case GLFW_KEY_SEMICOLON:
			return vkl::Key::KEY_SEMICOLON;
		case GLFW_KEY_EQUAL:
			return vkl::Key::KEY_EQUAL;
		case GLFW_KEY_A:
			return vkl::Key::KEY_A;
		case GLFW_KEY_B:
			return vkl::Key::KEY_B;
		case GLFW_KEY_C:
			return vkl::Key::KEY_C;
		case GLFW_KEY_D:
			return vkl::Key::KEY_D;
		case GLFW_KEY_E:
			return vkl::Key::KEY_E;
		case GLFW_KEY_F:
			return vkl::Key::KEY_F;
		case GLFW_KEY_G:
			return vkl::Key::KEY_G;
		case GLFW_KEY_H:
			return vkl::Key::KEY_H;
		case GLFW_KEY_I:
			return vkl::Key::KEY_I;
		case GLFW_KEY_J:
			return vkl::Key::KEY_J;
		case GLFW_KEY_K:
			return vkl::Key::KEY_K;
		case GLFW_KEY_L:
			return vkl::Key::KEY_L;
		case GLFW_KEY_M:
			return vkl::Key::KEY_M;
		case GLFW_KEY_N:
			return vkl::Key::KEY_N;
		case GLFW_KEY_O:
			return vkl::Key::KEY_O;
		case GLFW_KEY_P:
			return vkl::Key::KEY_P;
		case GLFW_KEY_Q:
			return vkl::Key::KEY_Q;
		case GLFW_KEY_R:
			return vkl::Key::KEY_R;
		case GLFW_KEY_S:
			return vkl::Key::KEY_S;
		case GLFW_KEY_T:
			return vkl::Key::KEY_T;
		case GLFW_KEY_U:
			return vkl::Key::KEY_U;
		case GLFW_KEY_V:
			return vkl::Key::KEY_V;
		case GLFW_KEY_W:
			return vkl::Key::KEY_W;
		case GLFW_KEY_X:
			return vkl::Key::KEY_X;
		case GLFW_KEY_Y:
			return vkl::Key::KEY_Y;
		case GLFW_KEY_Z:
			return vkl::Key::KEY_Z;
		case GLFW_KEY_LEFT_BRACKET:
			return vkl::Key::KEY_LEFT_BRACKET;
		case GLFW_KEY_BACKSLASH:
			return vkl::Key::KEY_BACKSLASH;
		case GLFW_KEY_RIGHT_BRACKET:
			return vkl::Key::KEY_RIGHT_BRACKET;
		case GLFW_KEY_GRAVE_ACCENT:
			return vkl::Key::KEY_GRAVE_ACCENT;
		case GLFW_KEY_WORLD_1:
			return vkl::Key::KEY_WORLD_1;
		case GLFW_KEY_WORLD_2:
			return vkl::Key::KEY_WORLD_2;
		case GLFW_KEY_ESCAPE:
			return vkl::Key::KEY_ESCAPE;
		case GLFW_KEY_ENTER:
			return vkl::Key::KEY_ENTER;
		case GLFW_KEY_TAB:
			return vkl::Key::KEY_TAB;
		case GLFW_KEY_BACKSPACE:
			return vkl::Key::KEY_BACKSPACE;
		case GLFW_KEY_INSERT:
			return vkl::Key::KEY_INSERT;
		case GLFW_KEY_DELETE:
			return vkl::Key::KEY_DELETE;
		case GLFW_KEY_RIGHT:
			return vkl::Key::KEY_RIGHT;
		case GLFW_KEY_LEFT:
			return vkl::Key::KEY_LEFT;
		case GLFW_KEY_DOWN:
			return vkl::Key::KEY_DOWN;
		case GLFW_KEY_UP:
			return vkl::Key::KEY_UP;
		case GLFW_KEY_PAGE_UP:
			return vkl::Key::KEY_PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:
			return vkl::Key::KEY_PAGE_DOWN;
		case GLFW_KEY_HOME:
			return vkl::Key::KEY_HOME;
		case GLFW_KEY_END:
			return vkl::Key::KEY_END;
		case GLFW_KEY_CAPS_LOCK:
			return vkl::Key::KEY_CAPS_LOCK;
		case GLFW_KEY_SCROLL_LOCK:
			return vkl::Key::KEY_SCROLL_LOCK;
		case GLFW_KEY_NUM_LOCK:
			return vkl::Key::KEY_NUM_LOCK;
		case GLFW_KEY_PRINT_SCREEN:
			return vkl::Key::KEY_PRINT_SCREEN;
		case GLFW_KEY_PAUSE:
			return vkl::Key::KEY_PAUSE;
		case GLFW_KEY_F1:
			return vkl::Key::KEY_F1;
		case GLFW_KEY_F2:
			return vkl::Key::KEY_F2;
		case GLFW_KEY_F3:
			return vkl::Key::KEY_F3;
		case GLFW_KEY_F4:
			return vkl::Key::KEY_F4;
		case GLFW_KEY_F5:
			return vkl::Key::KEY_F5;
		case GLFW_KEY_F6:
			return vkl::Key::KEY_F6;
		case GLFW_KEY_F7:
			return vkl::Key::KEY_F7;
		case GLFW_KEY_F8:
			return vkl::Key::KEY_F8;
		case GLFW_KEY_F9:
			return vkl::Key::KEY_F9;
		case GLFW_KEY_F10:
			return vkl::Key::KEY_F10;
		case GLFW_KEY_F11:
			return vkl::Key::KEY_F11;
		case GLFW_KEY_F12:
			return vkl::Key::KEY_F12;
		case GLFW_KEY_F13:
			return vkl::Key::KEY_F13;
		case GLFW_KEY_F14:
			return vkl::Key::KEY_F14;
		case GLFW_KEY_F15:
			return vkl::Key::KEY_F15;
		case GLFW_KEY_F16:
			return vkl::Key::KEY_F16;
		case GLFW_KEY_F17:
			return vkl::Key::KEY_F17;
		case GLFW_KEY_F18:
			return vkl::Key::KEY_F18;
		case GLFW_KEY_F19:
			return vkl::Key::KEY_F19;
		case GLFW_KEY_F20:
			return vkl::Key::KEY_F20;
		case GLFW_KEY_F21:
			return vkl::Key::KEY_F21;
		case GLFW_KEY_F22:
			return vkl::Key::KEY_F22;
		case GLFW_KEY_F23:
			return vkl::Key::KEY_F23;
		case GLFW_KEY_F24:
			return vkl::Key::KEY_F24;
		case GLFW_KEY_F25:
			return vkl::Key::KEY_F25;
		case GLFW_KEY_KP_0:
			return vkl::Key::KEY_KP_0;
		case GLFW_KEY_KP_1:
			return vkl::Key::KEY_KP_1;
		case GLFW_KEY_KP_2:
			return vkl::Key::KEY_KP_2;
		case GLFW_KEY_KP_3:
			return vkl::Key::KEY_KP_3;
		case GLFW_KEY_KP_4:
			return vkl::Key::KEY_KP_4;
		case GLFW_KEY_KP_5:
			return vkl::Key::KEY_KP_5;
		case GLFW_KEY_KP_6:
			return vkl::Key::KEY_KP_6;
		case GLFW_KEY_KP_7:
			return vkl::Key::KEY_KP_7;
		case GLFW_KEY_KP_8:
			return vkl::Key::KEY_KP_8;
		case GLFW_KEY_KP_9:
			return vkl::Key::KEY_KP_9;
		case GLFW_KEY_KP_DECIMAL:
			return vkl::Key::KEY_KP_DECIMAL;
		case GLFW_KEY_KP_DIVIDE:
			return vkl::Key::KEY_KP_DIVIDE;
		case GLFW_KEY_KP_MULTIPLY:
			return vkl::Key::KEY_KP_MULTIPLY;
		case GLFW_KEY_KP_SUBTRACT:
			return vkl::Key::KEY_KP_SUBTRACT;
		case GLFW_KEY_KP_ADD:
			return vkl::Key::KEY_KP_ADD;
		case GLFW_KEY_KP_ENTER:
			return vkl::Key::KEY_KP_ENTER;
		case GLFW_KEY_KP_EQUAL:
			return vkl::Key::KEY_KP_EQUAL;
		case GLFW_KEY_LEFT_SHIFT:
			return vkl::Key::KEY_LEFT_SHIFT;
		case GLFW_KEY_LEFT_CONTROL:
			return vkl::Key::KEY_LEFT_CONTROL;
		case GLFW_KEY_LEFT_ALT:
			return vkl::Key::KEY_LEFT_ALT;
		case GLFW_KEY_LEFT_SUPER:
			return vkl::Key::KEY_LEFT_SUPER;
		case GLFW_KEY_RIGHT_SHIFT:
			return vkl::Key::KEY_RIGHT_SHIFT;
		case GLFW_KEY_RIGHT_CONTROL:
			return vkl::Key::KEY_RIGHT_CONTROL;
		case GLFW_KEY_RIGHT_ALT:
			return vkl::Key::KEY_RIGHT_ALT;
		case GLFW_KEY_RIGHT_SUPER:
			return vkl::Key::KEY_RIGHT_SUPER;
		case GLFW_KEY_MENU:
			return vkl::Key::KEY_MENU;
		}
		return vkl::Key::KEY_SPACE;
	}


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

	//Event Callbacks
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto userWindow = reinterpret_cast<vkl::WindowData*>(glfwGetWindowUserPointer(window));
		auto event = std::make_unique<vkl::WindowResizeEvent>();
		event->height = height;
		event->width = width;
		userWindow->events.push_back(std::move(event));
	}

	static void windowPosCallback(GLFWwindow* window, int xpos, int ypos) {
		auto userWindow = reinterpret_cast<vkl::WindowData*>(glfwGetWindowUserPointer(window));
		auto event = std::make_unique<vkl::WindowMoveEvent>();
		event->x = xpos;
		event->y = ypos;
		userWindow->events.push_back(std::move(event));
	}

	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos){
		auto userWindow = reinterpret_cast<vkl::WindowData*>(glfwGetWindowUserPointer(window));
		auto event = std::make_unique<vkl::MouseMoveEvent>();
		event->x = xpos;
		event->y = ypos;
		userWindow->events.push_back(std::move(event));
	}


	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		auto userWindow = reinterpret_cast<vkl::WindowData*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS)
		{
			auto event = std::make_unique<vkl::MouseDownEvent>();
			event->button = glfwMouseButtonToVkl(button);
			userWindow->events.push_back(std::move(event));
		}
		else if(action == GLFW_RELEASE)
		{
			auto event = std::make_unique<vkl::MouseUpEvent>();
			event->button = glfwMouseButtonToVkl(button);
			userWindow->events.push_back(std::move(event));
		}
	}


	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto userWindow = reinterpret_cast<vkl::WindowData*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS)
		{
			auto event = std::make_unique<vkl::KeyDownEvent>();
			event->key = glfwKeyToVkl(key);
			userWindow->events.push_back(std::move(event));
		}
		else if (action == GLFW_RELEASE)
		{
			auto event = std::make_unique<vkl::KeyUpEvent>();
			event->key = glfwKeyToVkl(key);
			userWindow->events.push_back(std::move(event));
		}
		else if (action == GLFW_REPEAT)
		{
			auto event = std::make_unique<vkl::KeyRepeatEvent>();
			event->key = glfwKeyToVkl(key);
			userWindow->events.push_back(std::move(event));
		}

	}


	static void charCallback(GLFWwindow* window, unsigned int codepoint) {
		auto userWindow = reinterpret_cast<vkl::WindowData*>(glfwGetWindowUserPointer(window));
		auto event = std::make_unique<vkl::CharEvent>();
		event->unicode = codepoint;
		userWindow->events.push_back(std::move(event));
	}

	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		auto userWindow = reinterpret_cast<vkl::WindowData*>(glfwGetWindowUserPointer(window));
		auto event = std::make_unique<vkl::ScrollEvent>();
		event->xOffset = xoffset;
		event->yOffset = yoffset;
		userWindow->events.push_back(std::move(event));
	}

}

namespace vkl
{
	Window::Window(Window&&) noexcept = default;
	Window& Window::operator=(Window&&) noexcept = default;

	Window::Window(int width, int height, const char* title)
	{
		if (!isGLFWInitialized())
			return;

		_windowData = std::make_unique<WindowData>();


		_windowData->window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwSetWindowUserPointer(_windowData->window, _windowData.get());

		glfwSetFramebufferSizeCallback(_windowData->window, framebufferResizeCallback);
		glfwSetWindowPosCallback(_windowData->window, windowPosCallback);
		glfwSetCursorPosCallback(_windowData->window, cursorPosCallback);
		glfwSetMouseButtonCallback(_windowData->window, mouseButtonCallback);
		glfwSetKeyCallback(_windowData->window, keyCallback);
		glfwSetCharCallback(_windowData->window, charCallback);
		glfwSetScrollCallback(_windowData->window, scrollCallback);

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

	void Window::clearLastFrame()
	{
		_windowData->events.clear();
	}

	void Window::pollEventsForAllWindows()
	{
		glfwPollEvents();
	}

	void Window::updateToThisFrame()
	{
		glfwGetWindowPos(_windowData->window, &_windowData->x, &_windowData->y);
		glfwGetWindowSize(_windowData->window, &_windowData->width, &_windowData->height);
		_windowData->focused = glfwGetWindowAttrib(_windowData->window, GLFW_FOCUSED);
	}

	std::span<std::unique_ptr<const Event>> Window::events() const
	{
		return _windowData->events;
	}

	void Window::cleanUp()
	{
		glfwDestroyWindow(_windowData->window);
		_windowData->window = nullptr;
	}

	void Window::cleanUpWindowSystem()
	{
		glfwTerminate();
	}

	bool Window::isFocused() const
	{
		return _windowData->focused;
	}

	int Window::width() const
	{
		return _windowData->width;
	}

	int Window::height() const
	{
		return _windowData->height;
	}

	int Window::x() const
	{
		return _windowData->x;
	}

	int Window::y() const
	{
		return _windowData->y;
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

#endif