// GLApp.cpp

module;

#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

module GLApp;

import <iostream>;
//import <thread>;

import Renderer;
import Scene;

GLApp::GLApp(WindowSize window_size, std::string title)
{
	glfwSetErrorCallback([](int error, const char * description)
		{
			std::cout << "GLFW Error: " << error << " " << description;
		});

	if (!glfwInit())
		return;
	m_intialized = true;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_window = glfwCreateWindow(window_size.m_width, window_size.m_height, title.c_str(), nullptr, nullptr);
	if (!m_window)
		return;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width, int height)
		{
			GLApp * app = static_cast<GLApp *>(glfwGetWindowUserPointer(window));
			app->OnWindowResize(WindowSize{ width, height });
		});
	glfwSetKeyCallback(m_window, [](GLFWwindow * window, int key, int scan_code, int action, int mods)
		{
			GLApp * app = static_cast<GLApp *>(glfwGetWindowUserPointer(window));
			app->OnKeyEvent(key, scan_code, action, mods);
		});

	OnWindowResize(window_size);
}

GLApp::~GLApp()
{
	if (IsInitialized())
		glfwTerminate();
}

void GLApp::Run()
{
	if (!IsInitialized() || !HasWindow())
		return;

	std::jthread update_render_loop([&window = m_window, &new_window_size = m_new_window_size, &input = m_input](std::stop_token s_token)
		{
			glfwMakeContextCurrent(window);

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				std::cout << "Failed to initialize OpenGL context" << std::endl;
				return;
			}

			Renderer renderer;
			renderer.Init();
			Scene scene(renderer);
			scene.Init();

			double last_update_time = glfwGetTime();

			while (!s_token.stop_requested())
			{
				std::optional<WindowSize> size = new_window_size.exchange(std::nullopt);
				if (size.has_value())
					renderer.ResizeViewport(size->m_width, size->m_height);

				double cur_time = glfwGetTime();
				double delta_time = cur_time - last_update_time;
				last_update_time = cur_time;

				scene.Update(delta_time, input);

				renderer.Render();

				glfwSwapBuffers(window);
			}
		});

	while (!glfwWindowShouldClose(m_window))
		glfwPollEvents(); // must only be called from main thread
}

void GLApp::OnWindowResize(WindowSize size)
{
	m_new_window_size.store(size);
}

void GLApp::OnKeyEvent(int key, int /*scan_code*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	if (action == GLFW_PRESS)
		m_input.SetKey(key, true /*pressed*/);
	else if (action == GLFW_RELEASE)
		m_input.SetKey(key, false /*pressed*/);
}
