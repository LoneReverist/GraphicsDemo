// GLApp.h

#pragma once

#include "Renderer.h"

struct GLFWwindow;

class GLApp
{
public:
	GLApp(int window_width, int window_height, std::string title);
	~GLApp();

	void Run();

	bool IsInitialized() const { return m_intialized; }
	bool HasWindow() const { return m_window != nullptr; }

private:
	void process_input();

private:
	bool m_intialized{ false };
	GLFWwindow * m_window{ nullptr };
	Renderer m_renderer;
};
