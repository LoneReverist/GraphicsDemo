// GlfwVkWindow.cpp

module;

#include <cstdint>
#include <functional>
#include <string>

// include vulkan before glfw so it knows what graphics api we're using
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

module DreamhearthWindow;

namespace Dreamhearth
{
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

			GLFWmonitor * monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode * mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate); // must only be called from main thread
		}
	}

	RenderContext Window::CreateRenderContext(WindowSize size) const
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

		return RenderContext{ size.width, size.height, m_title,
			 extension_count, extensions, create_surface_fn };
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
} // namespace Dreamhearth
