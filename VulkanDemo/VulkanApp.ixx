// VulkanApp.ixx

module;

#include <atomic>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module VulkanApp;

import Input;

export struct WindowSize
{
	int width = 0;
	int height = 0;

	auto operator<=>(WindowSize const &) const = default;
};

export class VulkanApp
{
public:
	explicit VulkanApp(WindowSize window_size_screen_coords, std::string const & title);
	~VulkanApp();

	void Run();

	bool IsInitialized() const { return m_initialized; }
	bool HasWindow() const { return m_window != nullptr; }

	void OnWindowResize(WindowSize size);
	void OnKeyEvent(int key, int scan_code, int action, int mods);

private:
	bool m_initialized = false;
	GLFWwindow * m_window = nullptr;
	std::string const m_title;

	std::atomic<WindowSize> m_window_size_pixels;
	Input m_input;
};
