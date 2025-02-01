// GraphicsApi.cpp

module;

#include <algorithm>
#include <iostream>
#include <optional>
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

	VkInstance create_vk_instance(
		std::string const & app_title,
		uint32_t extension_count,
		char const ** extensions,
		bool enable_validation_layers,
		std::vector<char const *> const & validation_layers)
	{
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

		if (enable_validation_layers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		}

		VkInstance instance = VK_NULL_HANDLE;
		VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
		if (result != VK_SUCCESS)
			std::cout << "Failed to create vulkan instance.";

		return instance;
	}

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics_family;

		bool IsComplete() const { return graphics_family.has_value(); }
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;
		
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		for (size_t i = 0; i < queue_families.size(); ++i)
		{
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphics_family = static_cast<uint32_t>(i);

			if (indices.IsComplete())
				break;
		}

		return indices;
	}

	bool device_is_suitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		// Just find a dedicated gpu for now
		if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return false;

		QueueFamilyIndices indices = find_queue_families(device);
		if (!indices.IsComplete())
			return false;

		return true;
	}

	VkPhysicalDevice pick_physical_device(VkInstance instance)
	{
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		if (device_count == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

		auto iter = std::ranges::find_if(devices, [](VkPhysicalDevice device) { return device_is_suitable(device); });
		if (iter == devices.end())
			throw std::runtime_error("Failed to find a suitable GPU!");

		return *iter;
	}

	VkDevice create_logical_device(VkPhysicalDevice physical_device, VkQueue * graphics_queue = nullptr)
	{
		QueueFamilyIndices indices = find_queue_families(physical_device);

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = indices.graphics_family.value(),
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = 0,
			.pEnabledFeatures = &deviceFeatures
		};

		VkDevice logical_device = VK_NULL_HANDLE;
		VkResult result = vkCreateDevice(physical_device, &createInfo, nullptr, &logical_device);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device!");

		if (graphics_queue)
			vkGetDeviceQueue(logical_device, indices.graphics_family.value(), 0, graphics_queue);

		return logical_device;
	}
}

GraphicsApi::GraphicsApi(std::string const & app_title, uint32_t extension_count, char const ** extensions)
{
	if (m_enable_validation_layers && !validation_layers_are_supported(m_validation_layers))
		throw std::runtime_error("Validation layers requested, but not available!");

	m_instance = create_vk_instance(app_title, extension_count, extensions, m_enable_validation_layers, m_validation_layers);
	if (m_instance == VK_NULL_HANDLE)
		return;

	m_physical_device = pick_physical_device(m_instance);
	if (m_physical_device == VK_NULL_HANDLE)
		return;

	m_logical_device = create_logical_device(m_physical_device, &m_graphics_queue);
}

GraphicsApi::~GraphicsApi()
{
	vkDestroyDevice(m_logical_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}
