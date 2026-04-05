// OpenGLApp.cpp

module;

#include <atomic>
#include <iostream>
#include <optional>
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

module OpenGLApp;

import GraphicsApi;
import Scene;

OpenGLApp::OpenGLApp(WindowSize window_size_screen_coords, std::string const & title)
	: m_title(title)
{
	glfwSetErrorCallback([](int error, const char * description)
		{
			std::cout << "GLFW Error: " << error << " " << description;
		});

	if (!glfwInit())
		return;
	m_initialized = true;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_window = glfwCreateWindow(
		window_size_screen_coords.width,
		window_size_screen_coords.height,
		title.c_str(),
		nullptr,
		nullptr);
	if (!m_window)
		return;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width_pixels, int height_pixels)
		{
			OpenGLApp * app = static_cast<OpenGLApp *>(glfwGetWindowUserPointer(window));
			app->OnWindowResize(WindowSize{ width_pixels, height_pixels });
		});
	glfwSetKeyCallback(m_window, [](GLFWwindow * window, int key, int scan_code, int action, int mods)
		{
			OpenGLApp * app = static_cast<OpenGLApp *>(glfwGetWindowUserPointer(window));
			app->OnKeyEvent(key, scan_code, action, mods);
		});

	int width_pixels = 0, height_pixels = 0;
	glfwGetFramebufferSize(m_window, &width_pixels, &height_pixels);
	m_window_size_pixels.store(WindowSize{ width_pixels, height_pixels });
}

OpenGLApp::~OpenGLApp()
{
	if (IsInitialized())
		glfwTerminate();
}

void OpenGLApp::Run()
{
	if (!IsInitialized() || !HasWindow())
		return;

	std::jthread update_render_loop([this](std::stop_token s_token)
		{
			glfwMakeContextCurrent(m_window);

			WindowSize size = m_window_size_pixels.load();

			GraphicsApi graphics_api{ reinterpret_cast<GraphicsApi::LoadProcFn *>(glfwGetProcAddress) };

			Scene scene{ graphics_api, m_title };
			scene.OnViewportResized(size.width, size.height);

			double last_update_time = glfwGetTime();

			while (!s_token.stop_requested())
			{
				double cur_time = glfwGetTime();
				double delta_time = cur_time - last_update_time;
				last_update_time = cur_time;

				scene.Update(delta_time, m_input);

				scene.Render();
				glfwSwapBuffers(m_window);

				WindowSize new_size = m_window_size_pixels.load();
				if (new_size != size)
				{
					graphics_api.SetViewport(new_size.width, new_size.height);
					scene.OnViewportResized(new_size.width, new_size.height);
					size = new_size;
				}
			}
		});

	while (!glfwWindowShouldClose(m_window))
		glfwPollEvents(); // must only be called from main thread
}

void OpenGLApp::OnWindowResize(WindowSize size_pixels)
{
	m_window_size_pixels.store(size_pixels);
}

void OpenGLApp::OnKeyEvent(int key, int /*scan_code*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	if (action == GLFW_PRESS)
		m_input.SetKey(key, true /*pressed*/);
	else if (action == GLFW_RELEASE)
		m_input.SetKey(key, false /*pressed*/);
}
