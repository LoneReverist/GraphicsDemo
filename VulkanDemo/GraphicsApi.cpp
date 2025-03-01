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

	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR & capabilities, int width, int height)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = {
				std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};

			return actualExtent;
		}
	}

	VkSwapchainKHR create_swap_chain(
		PhysicalDeviceInfo phys_device_info,
		int width,
		int height,
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
		VkExtent2D extent = choose_swap_extent(sws.capabilities, width, height);
		if (extent.width == 0 || extent.height == 0)
			return VK_NULL_HANDLE;

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
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create vulkan swap chain");

		vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, nullptr);
		out_swap_chain_images.resize(image_count);
		vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, out_swap_chain_images.data());

		out_swap_chain_image_format = surface_format.format;
		out_swap_chain_extent = extent;

		return swap_chain;
	}

	VkRenderPass create_render_pass(VkDevice logical_device, VkFormat swap_chain_image_format)
	{
		VkAttachmentDescription color_attachment{
			.format = swap_chain_image_format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference color_attachment_ref{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment_ref
		};

		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		};

		VkRenderPassCreateInfo render_pass_info{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &color_attachment,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &dependency
		};

		VkRenderPass render_pass = VK_NULL_HANDLE;
		VkResult result = vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &render_pass);
		if (result != VK_SUCCESS)
			std::cout << "Failed to create render pass" << std::endl;

		return render_pass;
	}

	std::vector<VkImageView> create_image_views(
		std::vector<VkImage> const & images,
		VkDevice logical_device,
		VkFormat swap_chain_image_format)
	{
		std::vector<VkImageView> image_views(images.size());

		std::ranges::transform(images, image_views.begin(),
			[logical_device, swap_chain_image_format](VkImage image)
			{
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = image;
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swap_chain_image_format;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				VkImageView image_view = VK_NULL_HANDLE;
				VkResult result = vkCreateImageView(logical_device, &createInfo, nullptr, &image_view);
				if (result != VK_SUCCESS)
					throw std::runtime_error("Failed to create vulkan image view for swap chain image");

				return image_view;
			});

		return image_views;
	}

	std::vector<VkFramebuffer> create_framebuffers(
		std::vector<VkImageView> image_views,
		VkDevice logical_device,
		VkRenderPass render_pass,
		VkExtent2D swap_chain_extent)
	{
		std::vector<VkFramebuffer> framebuffers(image_views.size());

		std::ranges::transform(image_views, framebuffers.begin(),
			[logical_device, render_pass, swap_chain_extent](VkImageView image_view)
			{
				VkImageView attachments[] = { image_view };

				VkFramebufferCreateInfo framebuffer_info{
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass = render_pass,
					.attachmentCount = 1,
					.pAttachments = attachments,
					.width = swap_chain_extent.width,
					.height = swap_chain_extent.height,
					.layers = 1
				};

				VkFramebuffer framebuffer = VK_NULL_HANDLE;
				VkResult result = vkCreateFramebuffer(logical_device, &framebuffer_info, nullptr, &framebuffer);
				if (result != VK_SUCCESS)
					throw std::runtime_error("Failed to create vulkan framebuffer");

				return framebuffer;
			});

		return framebuffers;
	}

	VkCommandPool create_command_pool(
		PhysicalDeviceInfo const & phys_device_info,
		VkDevice logical_device)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = phys_device_info.qfis.graphics_family.value();

		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkResult result = vkCreateCommandPool(logical_device, &poolInfo, nullptr, &command_pool);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create vulkan command pool");

		return command_pool;
	}

	template <uint32_t count>
	std::array<VkCommandBuffer, count> create_command_buffers(VkCommandPool command_pool, VkDevice logical_device)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = command_pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = count;

		std::array<VkCommandBuffer, count> command_buffers;
		VkResult result = vkAllocateCommandBuffers(logical_device, &allocInfo, command_buffers.data());
		if (result != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffers!");

		return command_buffers;
	}

	// VkSemaphore is used for synchronizing commands on the gpu
	template <uint32_t count>
	std::array<VkSemaphore, count> create_semaphores(VkDevice logical_device)
	{
		VkSemaphoreCreateInfo semaphore_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		std::array<VkSemaphore, count> semaphores;
		for (uint32_t i = 0; i < count; ++i)
		{
			VkResult result = vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &semaphores[i]);
			if (result != VK_SUCCESS)
				throw std::runtime_error("failed to create semaphore!");
		}

		return semaphores;
	}

	// VkFence is used for synchronizing the cpu with the gpu
	template <uint32_t count>
	std::array<VkFence, count> create_fences(VkDevice logical_device, bool create_signaled)
	{
		VkFenceCreateInfo fence_info{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = create_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags{}
		};

		std::array<VkFence, count> fences;
		for (uint32_t i = 0; i < count; ++i)
		{
			VkResult result = vkCreateFence(logical_device, &fence_info, nullptr, &fences[i]);
			if (result != VK_SUCCESS)
				throw std::runtime_error("failed to create fence!");
		}

		return fences;
	}

	uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
		{
			if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}
}

GraphicsApi::GraphicsApi(
	GLFWwindow * window,
	int width,
	int height,
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

	m_phys_device_info = pick_physical_device(m_instance, m_surface, m_device_extensions);
	if (m_phys_device_info.device == VK_NULL_HANDLE)
		return;

	m_logical_device = create_logical_device(m_phys_device_info, m_device_extensions, m_graphics_queue, m_present_queue);
	if (m_logical_device == VK_NULL_HANDLE)
		return;

	m_swap_chain = create_swap_chain(m_phys_device_info, width, height, m_surface, m_logical_device,
		m_swap_chain_images, m_swap_chain_image_format, m_swap_chain_extent);
	if (m_swap_chain == VK_NULL_HANDLE)
		return;

	m_render_pass = create_render_pass(m_logical_device, m_swap_chain_image_format);
	if (m_render_pass == VK_NULL_HANDLE)
		return;

	m_swap_chain_image_views = create_image_views(m_swap_chain_images, m_logical_device, m_swap_chain_image_format);
	m_swap_chain_framebuffers = create_framebuffers(
		m_swap_chain_image_views, m_logical_device, m_render_pass, m_swap_chain_extent);

	m_command_pool = create_command_pool(m_phys_device_info, m_logical_device);
	if (m_command_pool == VK_NULL_HANDLE)
		return;

	m_command_buffers = create_command_buffers<m_max_frames_in_flight>(m_command_pool, m_logical_device);
	if (std::ranges::find(m_command_buffers, VK_NULL_HANDLE) != m_command_buffers.end())
		return;

	m_image_available_semaphores = create_semaphores<m_max_frames_in_flight>(m_logical_device);
	m_render_finished_semaphores = create_semaphores<m_max_frames_in_flight>(m_logical_device);
	m_in_flight_fences = create_fences<m_max_frames_in_flight>(m_logical_device, true /*create_signaled*/);
}

GraphicsApi::~GraphicsApi()
{
	for (auto fence : m_in_flight_fences)
		vkDestroyFence(m_logical_device, fence, nullptr);
	for (auto semaphore : m_render_finished_semaphores)
		vkDestroySemaphore(m_logical_device, semaphore, nullptr);
	for (auto semaphore : m_image_available_semaphores)
		vkDestroySemaphore(m_logical_device, semaphore, nullptr);

	vkDestroyCommandPool(m_logical_device, m_command_pool, nullptr);

	DestroySwapChain();

	vkDestroyRenderPass(m_logical_device, m_render_pass, nullptr);
	vkDestroyDevice(m_logical_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void GraphicsApi::DestroySwapChain()
{
	for (auto framebuffer : m_swap_chain_framebuffers)
		vkDestroyFramebuffer(m_logical_device, framebuffer, nullptr);
	m_swap_chain_framebuffers.clear();

	for (auto image_view : m_swap_chain_image_views)
		vkDestroyImageView(m_logical_device, image_view, nullptr);
	m_swap_chain_image_views.clear();

	vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
	m_swap_chain = VK_NULL_HANDLE;
	m_swap_chain_images.clear();
	m_swap_chain_image_format = VK_FORMAT_UNDEFINED;
	m_swap_chain_extent = VkExtent2D{ 0, 0 };
}

void GraphicsApi::RecreateSwapChain(int width, int height)
{
	if (width == 0 || height == 0)
		return;

	vkDeviceWaitIdle(m_logical_device);

	DestroySwapChain();

	m_phys_device_info.sws_details = query_swap_chain_support(m_phys_device_info.device, m_surface);

	m_swap_chain = create_swap_chain(m_phys_device_info, width, height, m_surface, m_logical_device,
		m_swap_chain_images, m_swap_chain_image_format, m_swap_chain_extent);
	if (m_swap_chain == VK_NULL_HANDLE)
		return;

	m_swap_chain_image_views = create_image_views(m_swap_chain_images, m_logical_device, m_swap_chain_image_format);
	m_swap_chain_framebuffers = create_framebuffers(
		m_swap_chain_image_views, m_logical_device, m_render_pass, m_swap_chain_extent);
}

bool GraphicsApi::SwapChainIsValid() const
{
	return m_swap_chain != VK_NULL_HANDLE;
}

void GraphicsApi::DrawFrame(std::function<void()> render_fn, bool & out_swap_chain_out_of_date)
{
	vkWaitForFences(m_logical_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(
		m_logical_device,
		m_swap_chain,
		UINT64_MAX,
		m_image_available_semaphores[m_current_frame],
		VK_NULL_HANDLE,
		&m_current_image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		out_swap_chain_out_of_date = true;
		return;
	}
	else if (result == VK_SUBOPTIMAL_KHR)
	{
		out_swap_chain_out_of_date = true;
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// Only reset the fence if we are submitting work
	vkResetFences(m_logical_device, 1, &m_in_flight_fences[m_current_frame]);

	result = vkResetCommandBuffer(m_command_buffers[m_current_frame], 0);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to reset command buffer");

	render_fn();

	VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame] };

	VkSubmitInfo submit_info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_command_buffers[m_current_frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores
	};

	result = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer!");

	VkSwapchainKHR swap_chains[] = { m_swap_chain };

	VkPresentInfoKHR present_info{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swap_chains,
		.pImageIndices = &m_current_image_index
	};

	result = vkQueuePresentKHR(m_present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		out_swap_chain_out_of_date = true;
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image!");

	m_current_frame = (m_current_frame + 1) % m_max_frames_in_flight;
}

void GraphicsApi::WaitForLastFrame() const
{
	vkDeviceWaitIdle(m_logical_device);
}

VkResult GraphicsApi::CreateBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer & out_buffer,
	VkDeviceMemory & out_buffer_memory) const
{
	VkBufferCreateInfo buffer_info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	VkResult result = vkCreateBuffer(m_logical_device, &buffer_info, nullptr, &out_buffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create buffer" << std::endl;
		return result;
	}

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(m_logical_device, out_buffer, &mem_requirements);

	uint32_t mem_type_index = find_memory_type(
		m_phys_device_info.device,
		mem_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkMemoryAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = mem_type_index
	};

	// TODO: should use something like VulkanMemoryAllocator library for managing memory
	result = vkAllocateMemory(m_logical_device, &alloc_info, nullptr, &out_buffer_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to allocate buffer memory" << std::endl;
		return result;
	}

	vkBindBufferMemory(m_logical_device, out_buffer, out_buffer_memory, 0);
	return VK_SUCCESS;
}

void GraphicsApi::CopyBuffer(
	VkBuffer src_buffer,
	VkBuffer dst_buffer,
	VkDeviceSize size) const
{
	VkCommandBufferAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(m_logical_device, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(command_buffer, &begin_info);

	VkBufferCopy copy_region{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size
	};

	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer
	};

	vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphics_queue);

	vkFreeCommandBuffers(m_logical_device, m_command_pool, 1, &command_buffer);
}
