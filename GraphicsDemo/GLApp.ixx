// GLApp.ixx

module;

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module GLApp;

import <atomic>;
import <optional>;
import <string>;

export struct WindowSize
{
	int m_width{ 0 };
	int m_height{ 0 };
};

export class GLApp
{
public:
	GLApp(WindowSize window_size, std::string title);
	~GLApp();

	void Run();

	bool IsInitialized() const { return m_intialized; }
	bool HasWindow() const { return m_window != nullptr; }

	void OnWindowResize(WindowSize size);

private:
	void process_input() const;

private:
	bool m_intialized{ false };
	GLFWwindow * m_window{ nullptr };

	std::atomic<std::optional<WindowSize>> m_new_window_size;
};
