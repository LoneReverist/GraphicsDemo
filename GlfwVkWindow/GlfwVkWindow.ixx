// GlfwVkWindow.ixx

module;

#include <functional>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module DreamhearthWindow;

import Dreamhearth;

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
		using OnErrorFn = std::function<void(std::string)>;
		using OnSizeChangedFn = std::function<void(int, int)>;
		using OnScaleFactorChangedFn = std::function<void(float)>;
		using OnKeyEventFn = std::function<void(int, int, int, int)>;

	public:
		explicit Window(WindowSize window_size_screen_coords, std::string const & title, OnErrorFn on_error);
		~Window();

		bool IsValid() const { return m_glfw_initialized && m_window != nullptr; }

		WindowSize GetWindowSizePixels() const;
		float GetWindowScaleFactor() const;

		void SetOnError(OnErrorFn on_error);
		void SetOnSizeChanged(OnSizeChangedFn on_size_changed);
		void SetOnScaleFactorChanged(OnScaleFactorChangedFn on_scale_factor_changed);
		void SetOnKeyEvent(OnKeyEventFn on_key_event);

		GraphicsApi CreateRenderContext(WindowSize size) const;

		DrawFrameResult DrawFrame(GraphicsApi & graphics_api, std::function<void()> render_fn);

		bool ShouldClose() const;
		void PollEvents() const;

	private:
		void on_size_changed(int width_pixels, int height_pixels) { m_on_size_changed(width_pixels, height_pixels); }
		void on_scale_factor_changed(float scale_factor) { m_on_scale_factor_changed(scale_factor); }
		void on_key_event(int key, int scan_code, int action, int mods) { m_on_key_event(key, scan_code, action, mods); }

	private:
		bool m_glfw_initialized = false;
		GLFWwindow * m_window = nullptr;
		std::string const m_title;

		OnSizeChangedFn m_on_size_changed;
		OnScaleFactorChangedFn m_on_scale_factor_changed;
		OnKeyEventFn m_on_key_event;
	};
} // namespace Dreamhearth
