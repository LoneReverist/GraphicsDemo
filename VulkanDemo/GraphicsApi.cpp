// GraphicsApi.cpp

module;

#include <algorithm>
#include <iostream>
#include <ranges>

#include <vulkan/vulkan.h>

module GraphicsApi;

namespace
{
	bool validation_layers_are_supported(std::vector<char const *> const & desired_layers)
	{
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		return std::ranges::all_of(desired_layers, [&available_layers](char const * const layer)
			{
				auto iter = std::ranges::find_if(available_layers, [layer](VkLayerProperties const & layer_props)
					{
						return std::string_view{ layer } == layer_props.layerName;
					});
				return iter != available_layers.end();
			});
	}
}

GraphicsApi::GraphicsApi(std::string const & app_title, uint32_t extension_count, char const ** extensions)
{
	if (m_enable_validation_layers && !validation_layers_are_supported(m_validation_layers))
		throw std::runtime_error("validation layers requested, but not available!");

	VkApplicationInfo app_info{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = app_title.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo create_info{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = 0,
		.enabledExtensionCount = extension_count,
		.ppEnabledExtensionNames = extensions
	};

	if (m_enable_validation_layers)
	{
		create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
		create_info.ppEnabledLayerNames = m_validation_layers.data();
	}

	VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
	if (result != VK_SUCCESS)
		std::cout << "Failed to create vulkan instance.";
}

GraphicsApi::~GraphicsApi()
{
	vkDestroyInstance(m_instance, nullptr);
}
