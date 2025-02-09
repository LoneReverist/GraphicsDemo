// GraphicsApi.cpp

module;

#include <algorithm>
#include <iostream>
#include <limits>
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

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	struct PhysicalDeviceInfo
	{
		VkPhysicalDevice device;
		QueueFamilyIndices qfis;
		SwapChainSupportDetails sws_details;
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;
		
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		for (uint32_t i = 0; i < static_cast<uint32_t>(queue_families.size()); ++i)
		{
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphics_family = i;

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
			if (present_support)
				indices.present_family = i;

			if (indices.IsComplete())
				break;
		}

		return indices;
	}

	SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails sws_details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &sws_details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

		if (format_count != 0)
		{
			sws_details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, sws_details.formats.data());
		}

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

		if (present_mode_count != 0) {
			sws_details.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, sws_details.present_modes.data());
		}

		return sws_details;
	}

	bool device_supports_extensions(VkPhysicalDevice device, std::vector<const char *> const & required_extensions)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

		return std::ranges::all_of(required_extensions, [&available_extensions](char const * const extension)
			{
				auto iter = std::ranges::find_if(available_extensions,
					[extension](VkExtensionProperties const & extension_props)
					{
						return std::string_view{ extension } == extension_props.extensionName;
					});
				return iter != available_extensions.end();
			});
	}

	bool device_is_suitable(
		VkPhysicalDevice device,
		std::vector<const char *> const & device_extensions,
		VkSurfaceKHR surface,
		PhysicalDeviceInfo & out_device_info)
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		// Just find a dedicated gpu for now
		if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return false;

		QueueFamilyIndices qfis = find_queue_families(device, surface);
		if (!qfis.IsComplete())
			return false;

		if (!device_supports_extensions(device, device_extensions))
			return false;

		SwapChainSupportDetails swap_chain_support = query_swap_chain_support(device, surface);
		if (swap_chain_support.formats.empty() || swap_chain_support.present_modes.empty())
			return false;

		out_device_info = PhysicalDeviceInfo{ device, qfis, swap_chain_support };
		return true;
	}

	PhysicalDeviceInfo pick_physical_device(
		VkInstance instance,
		VkSurfaceKHR surface,
		std::vector<const char *> const & device_extensions)
	{
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		if (device_count == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

		PhysicalDeviceInfo phys_device_info;
		auto iter = std::ranges::find_if(devices,
			[&device_extensions, surface, &phys_device_info](VkPhysicalDevice device)
			{
				return device_is_suitable(device, device_extensions, surface, phys_device_info);
			});
		if (iter == devices.end())
			throw std::runtime_error("Failed to find a suitable GPU!");

		return phys_device_info;
	}

	VkDevice create_logical_device(
		PhysicalDeviceInfo phys_device_info,
		std::vector<const char *> const & device_extensions,
		VkQueue & out_graphics_queue,
		VkQueue & out_present_queue)
	{
		std::set<uint32_t> unique_queue_families = {
			phys_device_info.qfis.graphics_family.value(),
			phys_device_info.qfis.present_family.value()
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
			.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
			.ppEnabledExtensionNames = device_extensions.data(),
			.pEnabledFeatures = &deviceFeatures
		};

		VkDevice logical_device = VK_NULL_HANDLE;
		VkResult result = vkCreateDevice(phys_device_info.device, &createInfo, nullptr, &logical_device);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device!");

		vkGetDeviceQueue(logical_device, phys_device_info.qfis.graphics_family.value(), 0, &out_graphics_queue);
		vkGetDeviceQueue(logical_device, phys_device_info.qfis.present_family.value(), 0, &out_present_queue);

		return logical_device;
	}

	VkSurfaceFormatKHR choose_swap_surface_format(std::vector<VkSurfaceFormatKHR> const & available_formats)
	{
		constexpr VkSurfaceFormatKHR desired_format{
			.format = VK_FORMAT_B8G8R8A8_SRGB,
			.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};

		auto iter = std::ranges::find_if(available_formats, [&desired_format](VkSurfaceFormatKHR const & format)
			{ return format.format == desired_format.format && format.colorSpace == desired_format.colorSpace; });
		if (iter != available_formats.end())
			return desired_format;

		return available_formats[0];
	}

	VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> & available_present_modes)
	{
		constexpr VkPresentModeKHR desired_mode = VK_PRESENT_MODE_MAILBOX_KHR;

		auto iter = std::ranges::find(available_present_modes, desired_mode);
		if (iter != available_present_modes.end())
			return desired_mode;

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR & capabilities, GLFWwindow * window)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};

			return actualExtent;
		}
	}

	VkSwapchainKHR create_swap_chain(
		PhysicalDeviceInfo phys_device_info,
		GLFWwindow * window,
		VkSurfaceKHR surface,
		VkDevice logical_device,
		std::vector<VkImage> & out_swap_chain_images,
		VkFormat & out_swap_chain_image_format,
		VkExtent2D & out_swap_chain_extent)
	{
		QueueFamilyIndices & qfis = phys_device_info.qfis;
		SwapChainSupportDetails & sws = phys_device_info.sws_details;

		VkSurfaceFormatKHR surface_format = choose_swap_surface_format(sws.formats);
		VkPresentModeKHR present_mode = choose_swap_present_mode(sws.present_modes);
		VkExtent2D extent = choose_swap_extent(sws.capabilities, window);

		uint32_t image_count = sws.capabilities.minImageCount + 1;
		if (sws.capabilities.maxImageCount > 0)
			image_count = std::min(image_count, sws.capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR create_info{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = surface,
			.minImageCount = image_count,
			.imageFormat = surface_format.format,
			.imageColorSpace = surface_format.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.preTransform = sws.capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = present_mode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		uint32_t qfis_array[] = {
			qfis.graphics_family.value(),
			qfis.present_family.value()
		};
		if (qfis.graphics_family != qfis.present_family)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = qfis_array;
		}

		VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
		VkResult result = vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swap_chain);
		if (swap_chain == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to create vulkan swap chain");

		vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, nullptr);
		out_swap_chain_images.resize(image_count);
		vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, out_swap_chain_images.data());

		out_swap_chain_image_format = surface_format.format;
		out_swap_chain_extent = extent;

		return swap_chain;
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

	const std::vector<const char *> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	PhysicalDeviceInfo phys_device_info = pick_physical_device(m_instance, m_surface, device_extensions);
	m_physical_device = phys_device_info.device;
	if (m_physical_device == VK_NULL_HANDLE)
		return;

	m_logical_device = create_logical_device(phys_device_info, device_extensions, m_graphics_queue, m_present_queue);
	if (m_logical_device == VK_NULL_HANDLE)
		return;

	m_swap_chain = create_swap_chain(phys_device_info, window, m_surface, m_logical_device,
		m_swap_chain_images, m_swap_chain_image_format, m_swap_chain_extent);
}

GraphicsApi::~GraphicsApi()
{
	vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
	vkDestroyDevice(m_logical_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}
