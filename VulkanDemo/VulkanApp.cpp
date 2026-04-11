// VulkanApp.cpp

module;

#include <atomic>
#include <cstdint>
#include <iostream>
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

module VulkanApp;

import GraphicsApi;
import Renderer;
import Scene;

VulkanApp::VulkanApp(WindowSize window_size_screen_coords, std::string const & title)
	: m_title(title)
{
	glfwSetErrorCallback([](int error, const char * description)
		{
			std::cout << "GLFW Error: " << error << " " << description;
		});

	if (!glfwInit())
		return;
	m_initialized = true;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(
		window_size_screen_coords.width,
		window_size_screen_coords.height,
		m_title.c_str(),
		nullptr,
		nullptr);
	if (!m_window)
		return;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width_pixels, int height_pixels)
		{
			VulkanApp * app = static_cast<VulkanApp *>(glfwGetWindowUserPointer(window));
			app->OnWindowResize(WindowSize{ width_pixels, height_pixels });
		});
	glfwSetKeyCallback(m_window, [](GLFWwindow * window, int key, int scan_code, int action, int mods)
		{
			VulkanApp * app = static_cast<VulkanApp *>(glfwGetWindowUserPointer(window));
			app->OnKeyEvent(key, scan_code, action, mods);
		});

	int width_pixels = 0, height_pixels = 0;
	glfwGetFramebufferSize(m_window, &width_pixels, &height_pixels); // must only be called from main thread
	m_window_size_pixels.store(WindowSize{ width_pixels, height_pixels });
}

VulkanApp::~VulkanApp()
{
	if (IsInitialized())
		glfwTerminate();
}

void VulkanApp::Run()
{
	if (!IsInitialized() || !HasWindow())
		return;

	std::jthread update_render_loop([this](std::stop_token s_token)
		{
			WindowSize size = m_window_size_pixels.load();
			std::uint32_t extension_count = 0;
			const char ** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

			GraphicsApi graphics_api{
				m_window, size.width, size.height,
				m_title, extension_count, extensions };

			Scene scene{ graphics_api, m_title };
			scene.OnViewportResized(size.width, size.height);

			double last_update_time = glfwGetTime();

			while (!s_token.stop_requested())
			{
				double cur_time = glfwGetTime();
				double delta_time = cur_time - last_update_time;
				last_update_time = cur_time;

				scene.Update(delta_time, m_input);

				bool swap_chain_out_of_date = false;
				if (graphics_api.SwapChainIsValid())
					graphics_api.DrawFrame([&scene]() { scene.Render(); }, swap_chain_out_of_date);
				else
					swap_chain_out_of_date = true;

				WindowSize new_size = m_window_size_pixels.load();
				if (swap_chain_out_of_date || new_size != size)
				{
					graphics_api.RecreateSwapChain(new_size.width, new_size.height);
					scene.OnViewportResized(new_size.width, new_size.height);
					size = new_size;
				}
			}

			graphics_api.WaitForLastFrame();
		}); // the GraphicsApi and Scene are destroyed in the reverse order they were created

	while (!glfwWindowShouldClose(m_window))
		glfwPollEvents(); // must only be called from main thread
}

void VulkanApp::OnWindowResize(WindowSize size_pixels)
{
	m_window_size_pixels.store(size_pixels);
}

void VulkanApp::OnKeyEvent(int key, int /*scan_code*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	if (action == GLFW_PRESS)
		m_input.SetKey(key, true /*pressed*/);
	else if (action == GLFW_RELEASE)
		m_input.SetKey(key, false /*pressed*/);
}
