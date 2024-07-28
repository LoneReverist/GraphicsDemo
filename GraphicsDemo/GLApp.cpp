// GLApp.cpp

#include "stdafx.h"
#include "GLApp.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

GLApp::GLApp(int window_width, int window_height, std::string title)
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

	m_window = glfwCreateWindow(window_width, window_height, title.c_str(), nullptr, nullptr);
	if (!m_window)
		return;

	glfwMakeContextCurrent(m_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		m_window = nullptr;
		return;
	}

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width, int height)
		{
			GLApp * app = static_cast<GLApp *>(glfwGetWindowUserPointer(window));
			app->OnWindowResize(width, height);
		});

	m_renderer.Init();
	m_renderer.ResizeViewport(window_width, window_height);
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

	while (!glfwWindowShouldClose(m_window))
	{
		process_input();

		double cur_time = glfwGetTime();
		double delta_time = cur_time - m_time;
		m_time = cur_time;
		m_renderer.Update(delta_time);

		m_renderer.Render();

		glfwSwapBuffers(m_window);

		glfwPollEvents();
	}
}

void GLApp::OnWindowResize(int width, int height)
{
	m_renderer.ResizeViewport(width, height);
}

void GLApp::process_input() const
{
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);
}
