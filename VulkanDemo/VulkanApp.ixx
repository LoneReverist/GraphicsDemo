// VulkanApp.ixx

module;

export module VulkanApp;

import <atomic>;
import <optional>;
import <string>;

import Input;

export struct WindowSize
{
	int m_width{ 0 };
	int m_height{ 0 };
};

export class VulkanApp
{
public:
	VulkanApp(WindowSize window_size, std::string title);
	~VulkanApp();

	void Run();

	bool IsInitialized() const { return m_intialized; }
	bool HasWindow() const { return m_window != nullptr; }

	void OnWindowResize(WindowSize size);
	void OnKeyEvent(int key, int scan_code, int action, int mods);

private:
	bool m_intialized{ false };
	GLFWwindow * m_window{ nullptr };

	std::atomic<std::optional<WindowSize>> m_new_window_size;
	Input m_input;
};
