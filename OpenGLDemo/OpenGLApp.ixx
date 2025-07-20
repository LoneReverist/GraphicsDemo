// OpenGLApp.ixx

module;

#include <atomic>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module OpenGLApp;

import Input;

export struct WindowSize
{
	int m_width = 0;
	int m_height = 0;

	auto operator<=>(WindowSize const &) const = default;
};

export class OpenGLApp
{
public:
	OpenGLApp(WindowSize window_size, std::string title);
	~OpenGLApp();

	void Run();

	bool IsInitialized() const { return m_initialized; }
	bool HasWindow() const { return m_window != nullptr; }

	void OnWindowResize(WindowSize size);
	void OnKeyEvent(int key, int scan_code, int action, int mods);

private:
	bool m_initialized = false;
	GLFWwindow * m_window = nullptr;

	std::atomic<WindowSize> m_window_size;
	Input m_input;
};
