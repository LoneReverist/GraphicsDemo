// VulkanApp.cpp

module;

#include <iostream>
#include <thread>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

module VulkanApp;

import GraphicsApi;
import Renderer;
import Scene;

VulkanApp::VulkanApp(WindowSize window_size, std::string title)
	: m_title(title)
{
	glfwSetErrorCallback([](int error, const char * description)
		{
			std::cout << "GLFW Error: " << error << " " << description;
		});

	if (!glfwInit())
		return;
	m_intialized = true;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(window_size.m_width, window_size.m_height, m_title.c_str(), nullptr, nullptr);
	if (!m_window)
		return;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow * window, int width, int height)
		{
			VulkanApp * app = static_cast<VulkanApp *>(glfwGetWindowUserPointer(window));
			app->OnWindowResize(WindowSize{ width, height });
		});
	glfwSetKeyCallback(m_window, [](GLFWwindow * window, int key, int scan_code, int action, int mods)
		{
			VulkanApp * app = static_cast<VulkanApp *>(glfwGetWindowUserPointer(window));
			app->OnKeyEvent(key, scan_code, action, mods);
		});

	m_window_size.store(window_size);
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
			WindowSize size = m_window_size.load();
			uint32_t extension_count = 0;
			const char ** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

			GraphicsApi graphics_api{
				m_window, size.m_width, size.m_height,
				m_title, extension_count, extensions };

			Scene scene{ graphics_api };
			scene.Init();
			scene.OnViewportResized(size.m_width, size.m_height);

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

				WindowSize new_size = m_window_size.load();
				if (swap_chain_out_of_date || new_size != size)
				{
					graphics_api.RecreateSwapChain(new_size.m_width, new_size.m_height);
					scene.OnViewportResized(new_size.m_width, new_size.m_height);
					size = new_size;
				}
			}

			graphics_api.WaitForLastFrame();
		}); // the GraphicsApi, Renderer and Scene are destroyed in the reverse order they were created

	while (!glfwWindowShouldClose(m_window))
		glfwPollEvents(); // must only be called from main thread
}

void VulkanApp::OnWindowResize(WindowSize size)
{
	m_window_size.store(size);
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
