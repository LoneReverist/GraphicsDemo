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

	void OnWindowResize(int width, int height);

private:
	void process_input() const;

private:
	bool m_intialized{ false };
	GLFWwindow * m_window{ nullptr };
	Renderer m_renderer;

	double m_time{ 0.0 };
};
