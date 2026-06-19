// GlfwGLWindow.ixx

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

	struct WindowRect
	{
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;
	};

	export class Window
	{
	public:
		using OnErrorFn = std::function<void(std::string)>;
		using OnSizeChangedFn = std::function<void(int, int)>;
		using OnScaleFactorChangedFn = std::function<void(float)>;
		using OnKeyEventFn = std::function<void(int, int, int, int)>;
		using OnMouseButtonEventFn = std::function<void(int, int, int)>;
		using OnCursorPosFn = std::function<void(float, float)>;

	public:
		explicit Window(WindowSize window_size_screen_coords, std::string const & title, OnErrorFn on_error);
		~Window();

		bool IsValid() const { return m_glfw_initialized && m_window != nullptr; }

		WindowSize GetWindowSizePixels() const; // must only be called from main thread
		float GetWindowScaleFactor() const; // must only be called from main thread

		void SetOnError(OnErrorFn on_error);
		void SetOnSizeChanged(OnSizeChangedFn on_size_changed);
		void SetOnScaleFactorChanged(OnScaleFactorChangedFn on_scale_factor_changed);
		void SetOnKeyEvent(OnKeyEventFn on_key_event);
		void SetOnMouseButtonEvent(OnMouseButtonEventFn on_mouse_button_event);
		void SetOnCursorPos(OnCursorPosFn on_cursor_pos);

		void ToggleFullscreen(); // must only be called from main thread

		RenderContext CreateRenderContext(WindowSize size) const;

		DrawFrameResult DrawFrame(RenderContext & render_context, std::function<void()> render_fn) const;

		void SetShouldClose(bool should_close) const;
		bool ShouldClose() const;
		void PollEvents() const;

	private:
		void on_size_changed(int width_pixels, int height_pixels) { m_on_size_changed(width_pixels, height_pixels); }
		void on_scale_factor_changed(float scale_factor) { m_on_scale_factor_changed(scale_factor); }
		void on_key_event(int key, int scan_code, int action, int mods) { m_on_key_event(key, scan_code, action, mods); }
		void on_mouse_button_event(int button, int action, int mods) { m_on_mouse_button_event(button, action, mods); }
		void on_cursor_pos(float x_pixels, float y_pixels) { m_on_cursor_pos(x_pixels, y_pixels); }

	private:
		bool m_glfw_initialized = false;
		GLFWwindow * m_window = nullptr;
		std::string const m_title;

		bool m_is_fullscreen = false;
		WindowRect m_stored_win_rect;

		OnSizeChangedFn m_on_size_changed;
		OnScaleFactorChangedFn m_on_scale_factor_changed;
		OnKeyEventFn m_on_key_event;
		OnMouseButtonEventFn m_on_mouse_button_event;
		OnCursorPosFn m_on_cursor_pos;
	};
} // namespace Dreamhearth
