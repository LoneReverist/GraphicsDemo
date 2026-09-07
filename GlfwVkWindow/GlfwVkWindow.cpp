// GlfwVkWindow.cpp

module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <functional>
#include <string>
#include <utility>

// include vulkan before glfw so it knows what graphics api we're using
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

module DreamhearthWindow;

namespace Dreamhearth
{
	namespace
	{
		GLFWmonitor * get_window_monitor(GLFWwindow * window)
		{
			int window_x = 0, window_y = 0;
			int window_width = 0, window_height = 0;
			glfwGetWindowPos(window, &window_x, &window_y);
			glfwGetWindowSize(window, &window_width, &window_height);

			GLFWmonitor * best_monitor = glfwGetPrimaryMonitor();
			long long best_overlap_area = 0;
			int monitor_count = 0;
			GLFWmonitor ** monitors = glfwGetMonitors(&monitor_count);
			for (int i = 0; i < monitor_count; ++i)
			{
				int monitor_x = 0, monitor_y = 0;
				glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);
				const GLFWvidmode * mode = glfwGetVideoMode(monitors[i]);
				if (!mode)
					continue;

				const int overlap_width = std::max(0,
					std::min(window_x + window_width, monitor_x + mode->width) - std::max(window_x, monitor_x));
				const int overlap_height = std::max(0,
					std::min(window_y + window_height, monitor_y + mode->height) - std::max(window_y, monitor_y));
				const long long overlap_area = static_cast<long long>(overlap_width) * overlap_height;
				if (overlap_area > best_overlap_area)
				{
					best_overlap_area = overlap_area;
					best_monitor = monitors[i];
				}
			}

			return best_monitor;
		}

		std::pair<float, float> cursor_pos_to_framebuffer_pixels(GLFWwindow * window, double x, double y)
		{
			int window_width = 0, window_height = 0;
			int framebuffer_width = 0, framebuffer_height = 0;
			glfwGetWindowSize(window, &window_width, &window_height);
			glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

			if (window_width <= 0 || window_height <= 0)
				return { static_cast<float>(x), static_cast<float>(y) };

			return {
				static_cast<float>(x) * static_cast<float>(framebuffer_width) / static_cast<float>(window_width),
				static_cast<float>(y) * static_cast<float>(framebuffer_height) / static_cast<float>(window_height)
			};
		}
	}

	Window::Window(WindowSize window_size_screen_coords, std::string const & title, OnErrorFn on_error)
		: m_title(title)
	{
		SetOnError(on_error);

		if (!glfwInit())
			return;
		m_glfw_initialized = true;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#if defined(__linux__)
		// The Cosmic compositor has issues with glfw and causes crashes
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif

		m_window = glfwCreateWindow(
			window_size_screen_coords.width,
			window_size_screen_coords.height,
			m_title.c_str(),
			nullptr,
			nullptr);
		if (!m_window)
			return;

		WindowRect & r = m_stored_win_rect;
		glfwGetWindowPos(m_window, &r.x, &r.y);
		glfwGetWindowSize(m_window, &r.w, &r.h);

		glfwSetWindowUserPointer(m_window, this);
	}

	Window::~Window()
	{
		if (m_glfw_initialized)
			glfwTerminate();
	}

	void Window::SetIcon(int width, int height, unsigned char * rgba_pixels)
	{
		if (!IsValid() || width <= 0 || height <= 0 || !rgba_pixels)
			return;

		GLFWimage icon{ width, height, rgba_pixels };
		glfwSetWindowIcon(m_window, 1, &icon);
	}

	WindowSize Window::GetWindowSizePixels() const
	{
		int width_pixels = 0, height_pixels = 0;
		glfwGetFramebufferSize(m_window, &width_pixels, &height_pixels); // must only be called from main thread
		return WindowSize{ width_pixels, height_pixels };
	}

	float Window::GetWindowScaleFactor() const
	{
		float x_scale = 1.0f, y_scale = 1.0f;
		glfwGetWindowContentScale(m_window, &x_scale, &y_scale); // must only be called from main thread
		return y_scale; // assume x and y scale are the same
	}

	void Window::SetOnError(OnErrorFn on_error)
	{
		// the error callback might be called before the window has been created, so the
		// glfwSetWindowUserPointer approach doesn't work here, we need a static variable instead
		static OnErrorFn error_callback_fn;

		error_callback_fn = [on_error](std::string msg)
			{
				on_error(std::move(msg));
			};
		
		glfwSetErrorCallback([](int error, const char * description)
			{
				if (error_callback_fn)
					error_callback_fn("GLFW Error: " + std::to_string(error) + " " + description);
			});
	}

	void Window::SetOnSizeChanged(OnSizeChangedFn on_size_changed)
	{
		m_on_size_changed = std::move(on_size_changed);
		glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width_pixels, int height_pixels)
			{
				Window * self = static_cast<Window *>(glfwGetWindowUserPointer(window));
				self->on_size_changed(width_pixels, height_pixels);
			});
	}

	void Window::SetOnScaleFactorChanged(OnScaleFactorChangedFn on_scale_factor_changed)
	{
		m_on_scale_factor_changed = std::move(on_scale_factor_changed);
		glfwSetWindowContentScaleCallback(m_window, [](GLFWwindow * window, float x_scale, float y_scale)
			{
				Window * self = static_cast<Window *>(glfwGetWindowUserPointer(window));
				self->on_scale_factor_changed(y_scale); // assume x and y scale are the same
			});
	}

	void Window::SetOnKeyEvent(OnKeyEventFn on_key_event)
	{
		m_on_key_event = std::move(on_key_event);
		glfwSetKeyCallback(m_window, [](GLFWwindow * window, int key, int scan_code, int action, int mods)
			{
				Window * self = static_cast<Window *>(glfwGetWindowUserPointer(window));
				self->on_key_event(key, scan_code, action, mods);

				if (key == GLFW_KEY_ENTER && mods & GLFW_MOD_ALT && action == GLFW_PRESS)
					self->ToggleFullscreen();
			});
	}

	void Window::SetOnMouseButtonEvent(OnMouseButtonEventFn on_mouse_button_event)
	{
		m_on_mouse_button_event = std::move(on_mouse_button_event);
		glfwSetMouseButtonCallback(m_window, [](GLFWwindow * window, int button, int action, int mods)
			{
				Window * self = static_cast<Window *>(glfwGetWindowUserPointer(window));
				self->on_mouse_button_event(button, action, mods);
			});
	}

	void Window::SetOnCursorPos(OnCursorPosFn on_cursor_pos)
	{
		m_on_cursor_pos = std::move(on_cursor_pos);
		glfwSetCursorPosCallback(m_window, [](GLFWwindow * window, double x, double y)
			{
				Window * self = static_cast<Window *>(glfwGetWindowUserPointer(window));
				auto const [x_pixels, y_pixels] = cursor_pos_to_framebuffer_pixels(window, x, y);
				self->on_cursor_pos(x_pixels, y_pixels);
			});
	}

	void Window::SetOnFocusChanged(OnFocusChangedFn on_focus_changed)
	{
		m_on_focus_changed = std::move(on_focus_changed);
		glfwSetWindowFocusCallback(m_window, [](GLFWwindow * window, int focused)
			{
				Window * self = static_cast<Window *>(glfwGetWindowUserPointer(window));
				self->on_focus_changed(focused == GLFW_TRUE);
			});
	}

	void Window::ToggleFullscreen()
	{
		if (m_is_fullscreen)
		{
			m_is_fullscreen = false;

			WindowRect & r = m_stored_win_rect;
			glfwSetWindowMonitor(m_window, NULL, r.x, r.y, r.w, r.h, GLFW_DONT_CARE); // must only be called from main thread
		}
		else
		{
			m_is_fullscreen = true;

			WindowRect r;
			glfwGetWindowPos(m_window, &r.x, &r.y);
			glfwGetWindowSize(m_window, &r.w, &r.h);
			if (r.w > 0 && r.h > 0)
				m_stored_win_rect = r;

			GLFWmonitor * monitor = get_window_monitor(m_window);
			const GLFWvidmode * mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate); // must only be called from main thread
		}
	}

	std::expected<RenderContext, GraphicsError> Window::CreateRenderContext(
		WindowSize size,
		GraphicsDiagnosticFn on_diagnostic) const
	{
		std::uint32_t extension_count = 0;
		const char ** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

		auto create_surface_fn = [window = m_window](VkInstance instance) -> VkSurfaceKHR
		{
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
			if (result != VK_SUCCESS)
				throw GraphicsException("Failed to create vulkan surface.");
			return surface;
		};

		return RenderContext::Create(size.width, size.height, m_title,
			extension_count, extensions, create_surface_fn, std::move(on_diagnostic));
	}

	DrawFrameResult Window::DrawFrame(RenderContext & render_context, std::function<void()> render_fn) const
	{
		if (!render_context.SwapChainIsValid())
			return DrawFrameResult::SwapChainOutOfDate;

		return render_context.DrawFrame(render_fn);
	}

	void Window::SetShouldClose(bool should_close) const
	{
		glfwSetWindowShouldClose(m_window, should_close);
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_window);
	}

	void Window::PollEvents() const
	{
		glfwPollEvents(); // must only be called from main thread
	}

	void Window::WaitEvents(double timeout_seconds) const
	{
		glfwWaitEventsTimeout(timeout_seconds); // must only be called from main thread
	}

	void Window::WakeEventLoop() const
	{
		glfwPostEmptyEvent(); // may be called from any thread
	}
} // namespace Dreamhearth
