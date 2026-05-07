// GlfwGLWindow.ixx

module;

#include <atomic>
#include <functional>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module DreamhearthWindow;

import Dreamhearth;

import Input;

namespace Dreamhearth
{
	export struct WindowSize
	{
		int width = 0;
		int height = 0;

		auto operator<=>(WindowSize const &) const = default;
	};

	export class Window
	{
	public:
		Window(WindowSize window_size_screen_coords, std::string const & title);
		~Window();

		bool IsValid() const { return m_glfw_initialized && m_window != nullptr; }

		GraphicsApi CreateRenderContext() const;

		DrawFrameResult DrawFrame(GraphicsApi & graphics_api, std::function<void()> render_fn);
		void Run();

		void OnKeyEvent(int key, int scan_code, int action, int mods);

	private:
		bool m_glfw_initialized = false;
		GLFWwindow * m_window = nullptr;
		std::string const m_title;

		// synchronized across update/render thread and main event loop thread
		std::atomic<WindowSize> m_window_size_pixels;
		std::atomic<float> m_window_scale_factor = 1.0f;
		Input m_input;
	};
} // namespace Dreamhearth
