// GLApp.cpp

#include "stdafx.h"
#include "GLApp.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

namespace
{
	void error_callback(int error, const char * description)
	{
		std::cout << "GLFW Error: " << error << " " << description;
	}

	void framebuffer_size_callback(GLFWwindow * window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
}

GLApp::GLApp(int window_width, int window_height, std::string title)
{
	glfwSetErrorCallback(error_callback);

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

	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	glViewport(0, 0, window_width, window_height);

	m_renderer.Init();
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

		float time = static_cast<float>(glfwGetTime());
		m_renderer.Update(time);

		m_renderer.Render();

		glfwSwapBuffers(m_window);

		glfwPollEvents();
	}
}

void GLApp::process_input()
{
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);
}
