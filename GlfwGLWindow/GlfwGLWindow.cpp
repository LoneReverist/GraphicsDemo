// GlfwGLWindow.cpp

module;

#include <atomic>
#include <functional>
#include <iostream>
#include <optional>
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

module DreamhearthWindow;

import Dreamhearth;

import Scene;

namespace Dreamhearth
{
	Window::Window(WindowSize window_size_screen_coords, std::string const & title)
		: m_title(title)
	{
		glfwSetErrorCallback([](int error, const char * description)
			{
				std::cout << "GLFW Error: " << error << " " << description << std::endl;
			});

		if (!glfwInit())
			return;
		m_glfw_initialized = true;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(__linux__)
		// The Cosmic compositor has issues with glfw and causes crashes
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif

		m_window = glfwCreateWindow(
			window_size_screen_coords.width,
			window_size_screen_coords.height,
			title.c_str(),
			nullptr,
			nullptr);
		if (!m_window)
			return;

		int width_pixels = 0, height_pixels = 0;
		glfwGetFramebufferSize(m_window, &width_pixels, &height_pixels); // must only be called from main thread
		m_window_size_pixels.store(WindowSize{ width_pixels, height_pixels });

		float x_scale = 1.0f, y_scale = 1.0f;
		glfwGetWindowContentScale(m_window, &x_scale, &y_scale); // must only be called from main thread
		m_window_scale_factor.store(y_scale); // assume x and y scale are the same

		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width_pixels, int height_pixels)
			{
				Window * win = static_cast<Window *>(glfwGetWindowUserPointer(window));
				win->m_window_size_pixels.store(WindowSize{ width_pixels, height_pixels });
			});
		glfwSetWindowContentScaleCallback(m_window, [](GLFWwindow * window, float x_scale, float y_scale)
			{
				Window * win = static_cast<Window *>(glfwGetWindowUserPointer(window));
				win->m_window_scale_factor.store(y_scale); // assume x and y scale are the same
			});
		glfwSetKeyCallback(m_window, [](GLFWwindow * window, int key, int scan_code, int action, int mods)
			{
				Window * win = static_cast<Window *>(glfwGetWindowUserPointer(window));
				win->OnKeyEvent(key, scan_code, action, mods);
			});
	}

	Window::~Window()
	{
		if (m_glfw_initialized)
			glfwTerminate();
	}

	GraphicsApi Window::CreateRenderContext() const
	{
		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(0); // vsync is sometimes on by default, disable it for more accurate timing measurements

		return GraphicsApi{ reinterpret_cast<GraphicsApi::LoadProcFn *>(glfwGetProcAddress) };
	}

	DrawFrameResult Window::DrawFrame(GraphicsApi & /*graphics_api*/, std::function<void()> render_fn)
	{
		render_fn();
		glfwSwapBuffers(m_window);
		return DrawFrameResult::Success;
	}

	void Window::Run()
	{
		if (!IsValid())
			return;

		std::jthread update_render_loop([this](std::stop_token s_token)
			{
				GraphicsApi graphics_api = CreateRenderContext();

				WindowSize size = m_window_size_pixels.load();
				float scale_factor = m_window_scale_factor.load();

				Scene scene{ graphics_api, m_title, scale_factor };
				scene.OnViewportResized(size.width, size.height);

				double last_update_time = glfwGetTime();

				while (!s_token.stop_requested())
				{
					double cur_time = glfwGetTime();
					double delta_time = cur_time - last_update_time;
					last_update_time = cur_time;

					scene.Update(delta_time, m_input);

					DrawFrame(graphics_api, [&scene]() { scene.Render(); });

					WindowSize new_size = m_window_size_pixels.load();
					if (new_size != size)
					{
						graphics_api.SetViewport(new_size.width, new_size.height);
						scene.OnViewportResized(new_size.width, new_size.height);
						size = new_size;
					}
					float new_scale_factor = m_window_scale_factor.load();
					if (new_scale_factor != scale_factor)
					{
						scene.OnDPIScalingFactorChanged(new_scale_factor);
						scale_factor = new_scale_factor;
					}
				}
			});

		while (!glfwWindowShouldClose(m_window))
			glfwPollEvents(); // must only be called from main thread
	}

	void Window::OnKeyEvent(int key, int /*scan_code*/, int action, int /*mods*/)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(m_window, true);

		if (action == GLFW_PRESS)
			m_input.SetKey(key, true /*pressed*/);
		else if (action == GLFW_RELEASE)
			m_input.SetKey(key, false /*pressed*/);
	}
} // namespace Dreamhearth
