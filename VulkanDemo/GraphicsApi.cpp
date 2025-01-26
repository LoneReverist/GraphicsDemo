// GraphicsApi.cpp

module;

#include <vulkan/vulkan.h>

module GraphicsApi;

import <iostream>;

GraphicsApi::GraphicsApi(std::string const & app_title, uint32_t extension_count, char const ** extensions)
{
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = app_title.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.enabledExtensionCount = extension_count,
		.ppEnabledExtensionNames = extensions
	};

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
	if (result != VK_SUCCESS)
		std::cout << "Failed to create vulkan instance.";
}

GraphicsApi::~GraphicsApi()
{
	vkDestroyInstance(m_instance, nullptr);
}
