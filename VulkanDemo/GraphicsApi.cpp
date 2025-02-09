// GraphicsApi.cpp

module;

#include <algorithm>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
				auto iter = std::ranges::find_if(available_layers,
					[layer](VkLayerProperties const & layer_props)
					{
						return std::string_view{ layer } == layer_props.layerName;
					});
				return iter != available_layers.end();
			});
	}

	VkInstance create_instance(
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

	VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow * window)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
		if (result != VK_SUCCESS)
			std::cout << "Failed to create vulkan surface.";

		return surface;
	}

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		bool IsComplete() const { return graphics_family.has_value() && present_family.has_value(); }
	};

	struct PhysicalDeviceAndQFIs
	{
		VkPhysicalDevice device;
		QueueFamilyIndices qfi;
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
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

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
			if (present_support)
				indices.present_family = i;

			if (indices.IsComplete())
				break;
		}

		return indices;
	}

	bool device_is_suitable(PhysicalDeviceAndQFIs const & phys_dq)
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(phys_dq.device, &device_properties);

		// Just find a dedicated gpu for now
		if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return false;

		if (!phys_dq.qfi.IsComplete())
			return false;

		return true;
	}

	PhysicalDeviceAndQFIs pick_physical_device(VkInstance instance, VkSurfaceKHR surface)
	{
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		if (device_count == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

		std::vector<PhysicalDeviceAndQFIs> phys_dqs(devices.size());
		std::ranges::transform(devices, phys_dqs.begin(), [surface](VkPhysicalDevice device)
			{ return PhysicalDeviceAndQFIs{ device, find_queue_families(device, surface) }; });

		auto iter = std::ranges::find_if(phys_dqs,
			[](PhysicalDeviceAndQFIs const & phys_dq) { return device_is_suitable(phys_dq); });
		if (iter == phys_dqs.end())
			throw std::runtime_error("Failed to find a suitable GPU!");

		return *iter;
	}

	VkDevice create_logical_device(
		PhysicalDeviceAndQFIs phys_dq,
		VkQueue * graphics_queue = nullptr,
		VkQueue * present_queue = nullptr)
	{
		std::set<uint32_t> unique_queue_families = {
			phys_dq.qfi.graphics_family.value(),
			phys_dq.qfi.present_family.value()
		};
		
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos(unique_queue_families.size());
		std::ranges::transform(unique_queue_families, queue_create_infos.begin(),
			[](uint32_t qfi)
			{
				constexpr float queue_priority = 1.0f;
				return VkDeviceQueueCreateInfo{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = qfi,
					.queueCount = 1,
					.pQueuePriorities = &queue_priority
				};
			});

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
			.pQueueCreateInfos = queue_create_infos.data(),
			.enabledExtensionCount = 0,
			.pEnabledFeatures = &deviceFeatures
		};

		VkDevice logical_device = VK_NULL_HANDLE;
		VkResult result = vkCreateDevice(phys_dq.device, &createInfo, nullptr, &logical_device);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device!");

		if (graphics_queue)
			vkGetDeviceQueue(logical_device, phys_dq.qfi.graphics_family.value(), 0, graphics_queue);
		if (present_queue)
			vkGetDeviceQueue(logical_device, phys_dq.qfi.present_family.value(), 0, present_queue);

		return logical_device;
	}
}

GraphicsApi::GraphicsApi(
	GLFWwindow * window,
	std::string const & app_title,
	uint32_t extension_count,
	char const ** extensions)
{
	if (m_enable_validation_layers && !validation_layers_are_supported(m_validation_layers))
		throw std::runtime_error("Validation layers requested, but not available!");

	m_instance = create_instance(app_title, extension_count, extensions, m_enable_validation_layers, m_validation_layers);
	if (m_instance == VK_NULL_HANDLE)
		return;

	m_surface = create_surface(m_instance, window);
	if (m_surface == VK_NULL_HANDLE)
		return;

	PhysicalDeviceAndQFIs phys_dq = pick_physical_device(m_instance, m_surface);
	m_physical_device = phys_dq.device;
	if (m_physical_device == VK_NULL_HANDLE)
		return;

	m_logical_device = create_logical_device(phys_dq, &m_graphics_queue, &m_present_queue);
}

GraphicsApi::~GraphicsApi()
{
	vkDestroyDevice(m_logical_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}
