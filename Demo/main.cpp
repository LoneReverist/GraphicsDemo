// main.cpp

#include <atomic>
#include <iostream>
#include <thread>
#include <utility>

import Dreamhearth;
import DreamhearthWindow;

import Input;
import Scene;

#ifdef BUILD_VULKAN
constexpr char const * AppName = "Vulkan Demo - Dreamhearth Engine";
#elif defined(BUILD_OPENGL)
constexpr char const * AppName = "OpenGL Demo - Dreamhearth Engine";
#else
#error "Either BUILD_VULKAN or BUILD_OPENGL must be defined"
#endif

namespace dh = Dreamhearth;

void on_error(std::string msg)
{
	std::cout << msg << std::endl;
}

void on_graphics_diagnostic(dh::GraphicsDiagnostic const & diagnostic)
{
	on_error(diagnostic.message);
}

int main()
{
	std::cout << "Initializing app..." << std::endl;

	dh::Window window(dh::WindowSize{ 1920, 1080 }, AppName, on_error);
	if (!window.IsValid())
		return -1;

	// these objects are synchronized across update/render thread and main event loop thread
	Input input;
	std::atomic<dh::WindowSize> window_size_pixels = window.GetWindowSizePixels(); // must only be called from main thread
	std::atomic<float> window_scale_factor = window.GetWindowScaleFactor(); // must only be called from main thread

	window.SetOnSizeChanged([&window_size_pixels](int width_pixels, int height_pixels)
		{
			window_size_pixels.store(dh::WindowSize{ width_pixels, height_pixels });
		});
	window.SetOnScaleFactorChanged([&window_scale_factor](float scale_factor)
		{
			window_scale_factor.store(scale_factor);
		});
	window.SetOnKeyEvent([&input](int key, int /*scan_code*/, int action, int /*mods*/)
		{
			if (static_cast<Input::Action>(action) == Input::Action::Press)
				input.SetKey(key, true /*pressed*/);
			else if (static_cast<Input::Action>(action) == Input::Action::Release)
				input.SetKey(key, false /*pressed*/);
		});

	std::cout << "Running app..." << std::endl;

	std::jthread update_render_loop(
		[&window, &input, &window_size_pixels, &window_scale_factor](std::stop_token s_token)
		{
			dh::WindowSize last_window_size = window_size_pixels.load();
			float scale_factor = window_scale_factor.load();

			auto render_context_result = window.CreateRenderContext(last_window_size, on_graphics_diagnostic);
			if (!render_context_result)
			{
				on_error(render_context_result.error().GetMessage());
				window.SetShouldClose(true);
				return;
			}
			dh::RenderContext render_context = std::move(render_context_result).value();

			Scene scene{ render_context, AppName, scale_factor };
			dh::RenderExtent render_extent = render_context.GetSwapChainExtent();
			scene.OnWindowResized(render_extent.width, render_extent.height);

			auto last_update_time = std::chrono::steady_clock::now();
			bool swap_chain_needs_recreation = false;

			while (!s_token.stop_requested())
			{
				dh::WindowSize const window_size = window_size_pixels.load();
				if (swap_chain_needs_recreation || window_size != last_window_size)
				{
					last_window_size = window_size;
					auto recreate_result = render_context.RecreateSwapChain(window_size.width, window_size.height);
					if (!recreate_result)
					{
						on_error(recreate_result.error().GetMessage());
						break;
					}

					render_extent = render_context.GetSwapChainExtent();
					if (render_extent.width > 0 && render_extent.height > 0)
					{
						scene.OnWindowResized(render_extent.width, render_extent.height);
						swap_chain_needs_recreation = false;
					}
					else
					{
						swap_chain_needs_recreation = true;
					}
				}

				if (render_extent.width == 0 || render_extent.height == 0)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds{ 16 });
					last_update_time = std::chrono::steady_clock::now();
					continue;
				}

				auto cur_time = std::chrono::steady_clock::now();
				float dt = std::chrono::duration<float>(cur_time - last_update_time).count(); // seconds
				last_update_time = cur_time;

				if (!scene.Update(dt, input))
					break;

				dh::DrawFrameResult draw_result = window.DrawFrame(render_context, [&scene]() { scene.Render(); });

				if (draw_result == dh::DrawFrameResult::SurfaceLost)
					break; // The Cosmic compositor has issues

				if (draw_result == dh::DrawFrameResult::SwapChainOutOfDate)
					swap_chain_needs_recreation = true;

				float new_scale_factor = window_scale_factor.load();
				if (new_scale_factor != scale_factor)
				{
					scene.OnDPIScalingFactorChanged(new_scale_factor);
					scale_factor = new_scale_factor;
				}
			}

			render_context.WaitForLastFrame();
			window.SetShouldClose(true); // signal main thread to exit
		}); // the RenderContext and Scene are destroyed in the reverse order they were created

	while (!window.ShouldClose())
		window.PollEvents(); // must only be called from main thread
}
