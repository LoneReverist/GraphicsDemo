// RenderContext.cpp

module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <functional>
#include <limits>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

module Dreamhearth;

import :RenderContext;
import :GraphicsError;
import :PhysicalDevice;

namespace Dreamhearth
{
	bool validation_layers_are_supported(
		vk::raii::Context const & context,
		std::vector<char const *> const & desired_layers)
	{
		auto layer_props = context.enumerateInstanceLayerProperties();
		auto unsupported_layer_iter = std::ranges::find_if(desired_layers,
			[&layer_props](auto const & desired_layer)
			{
				return std::ranges::none_of(layer_props,
					[desired_layer](auto const & layerProperty)
					{
						return strcmp(layerProperty.layerName, desired_layer) == 0;
					});
			});
		return unsupported_layer_iter == desired_layers.end();
	}

	vk::raii::Instance create_instance(
		vk::raii::Context const & context,
		std::string const & app_title,
		std::uint32_t extension_count,
		char const ** extensions,
		bool enable_validation_layers,
		std::vector<char const *> const & validation_layers)
	{
		vk::ApplicationInfo app_info{
			.pApplicationName = app_title.c_str(),
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = vk::ApiVersion13
		};

		vk::InstanceCreateInfo create_info{
			.pApplicationInfo = &app_info,
			.enabledLayerCount = 0,
			.enabledExtensionCount = extension_count,
			.ppEnabledExtensionNames = extensions
		};

		if (enable_validation_layers)
		{
			create_info.enabledLayerCount = static_cast<std::uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		}

		return vk::raii::Instance{ context, create_info };
	}

	vk::raii::Device create_logical_device(
		PhysicalDeviceInfo const & phys_device_info,
		std::vector<const char *> const & device_extensions)
	{
		float queue_priority = 1.0f;
		vk::DeviceQueueCreateInfo queue_create_info{
			.queueFamilyIndex = phys_device_info.queue_index,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority
		};

		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan13Features>
			feature_chain =
		{
			{
				.features = {.samplerAnisotropy = VK_TRUE }
			},
			{
				.synchronization2 = true,
				.dynamicRendering = true,      // Enable dynamic rendering from Vulkan 1.3
			}
		};

		vk::DeviceCreateInfo create_info{
			.pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queue_create_info,
			.enabledExtensionCount = static_cast<std::uint32_t>(device_extensions.size()),
			.ppEnabledExtensionNames = device_extensions.data(),
		};

		return vk::raii::Device{ phys_device_info.device, create_info };
	}

	vk::SurfaceFormatKHR choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const & available_formats)
	{
		constexpr vk::SurfaceFormatKHR desired_format{
			.format = vk::Format::eB8G8R8A8Srgb,
			.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
		};

		const auto iter = std::ranges::find_if(available_formats, [&desired_format](vk::SurfaceFormatKHR const & format)
			{ return format.format == desired_format.format && format.colorSpace == desired_format.colorSpace; });
		if (iter != available_formats.end())
			return *iter;

		return available_formats[0];
	}

	vk::PresentModeKHR choose_swap_present_mode(std::vector<vk::PresentModeKHR> const &)
	{
//		constexpr vk::PresentModeKHR desired_mode = vk::PresentModeKHR::eMailbox;
//
//		auto iter = std::ranges::find(available_present_modes, desired_mode);
//		if (iter != available_present_modes.end())
//			return *iter;

		// FIFO is guaranteed by Vulkan and paces rendering to the display refresh.
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D choose_swap_extent(vk::SurfaceCapabilitiesKHR const & capabilities, int width_pixels, int height_pixels)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
			return capabilities.currentExtent;

		return {
			std::clamp<std::uint32_t>(width_pixels, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<std::uint32_t>(height_pixels, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	}

	std::uint32_t choose_swap_min_image_count(vk::SurfaceCapabilitiesKHR const & capabilities)
	{
		std::uint32_t min_image_count = std::max(3u, capabilities.minImageCount);

		if (capabilities.maxImageCount > 0 && min_image_count > capabilities.maxImageCount)
			min_image_count = capabilities.maxImageCount;

		return min_image_count;
	}

	vk::raii::SwapchainKHR create_swap_chain(
		PhysicalDeviceInfo const & phys_device_info,
		int width_pixels,
		int height_pixels,
		vk::raii::SurfaceKHR const & surface,
		vk::raii::Device const & device,
		vk::Format & out_swap_chain_image_format,
		vk::Extent2D & out_swap_chain_extent)
	{
		SwapChainSupportDetails const & sws = phys_device_info.sws_details;

		vk::SurfaceFormatKHR surface_format = choose_swap_surface_format(sws.formats);
		vk::PresentModeKHR present_mode = choose_swap_present_mode(sws.present_modes);
		vk::Extent2D extent = choose_swap_extent(sws.capabilities, width_pixels, height_pixels);
		if (extent.width == 0 || extent.height == 0)
			throw GraphicsException("Failed to choose swap extent, got 0 for width or height");

		std::uint32_t min_image_count = choose_swap_min_image_count(sws.capabilities);

		vk::SwapchainCreateInfoKHR create_info{
			.surface = *surface,
			.minImageCount = min_image_count,
			.imageFormat = surface_format.format,
			.imageColorSpace = surface_format.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.preTransform = sws.capabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = present_mode,
			.clipped = vk::True,
		};

		out_swap_chain_image_format = surface_format.format;
		out_swap_chain_extent = extent;

		return vk::raii::SwapchainKHR{ device, create_info };
	}

	std::vector<vk::raii::ImageView> create_swap_chain_image_views(
		vk::raii::Device const & device,
		std::vector<vk::Image> const & images,
		vk::Format image_format)
	{
		std::vector<vk::raii::ImageView> image_views;
		image_views.reserve(images.size());

		vk::ImageViewCreateInfo create_info{
			.viewType = vk::ImageViewType::e2D,
			.format = image_format,
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.levelCount = 1,
				.layerCount = 1
			}
		};

		for (auto & image : images)
		{
			create_info.image = image;
			image_views.emplace_back(device, create_info);
		}
		return image_views;
	}

	vk::Format find_supported_format(
		vk::raii::PhysicalDevice const & phys_device,
		std::vector<vk::Format> const & candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features)
	{
		for (vk::Format format : candidates)
		{
			vk::FormatProperties props = phys_device.getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw GraphicsException("Failed to find supported format");
	}

	vk::Format find_depth_image_format(vk::raii::PhysicalDevice const & phys_device)
	{
		return find_supported_format(
			phys_device,
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	bool has_stencil_component(vk::Format format)
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	vk::raii::CommandPool create_command_pool(
		PhysicalDeviceInfo const & phys_device_info,
		vk::raii::Device const & device)
	{
		vk::CommandPoolCreateInfo pool_info{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = phys_device_info.queue_index
		};

		return vk::raii::CommandPool{ device, pool_info };
	}

	vk::raii::CommandBuffers create_command_buffers(
		vk::raii::Device const & device,
		vk::raii::CommandPool const & command_pool,
		std::uint32_t count)
	{
		vk::CommandBufferAllocateInfo alloc_info{
			.commandPool = command_pool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = count
		};

		return vk::raii::CommandBuffers{ device, alloc_info };
	}

	std::expected<RenderContext, GraphicsError> RenderContext::Create(
		int width_pixels,
		int height_pixels,
		std::string const & app_title,
		std::uint32_t extension_count,
		char const ** extensions,
		CreateSurfaceFn create_surface_fn,
		GraphicsDiagnosticFn on_diagnostic)
	{
		try
		{
			return RenderContext{
				width_pixels,
				height_pixels,
				app_title,
				extension_count,
				extensions,
				std::move(create_surface_fn),
				std::move(on_diagnostic)
			};
		}
		catch (vk::SystemError const & err)
		{
			return std::unexpected{ GraphicsError{ "Vulkan error: " + std::string(err.what()) } };
		}
		catch (GraphicsException const & err)
		{
			return std::unexpected{ GraphicsError{ "Graphics error: " + std::string(err.what()) } };
		}
	}

	RenderContext::RenderContext(
		int width_pixels,
		int height_pixels,
		std::string const & app_title,
		std::uint32_t extension_count,
		char const ** extensions,
		CreateSurfaceFn create_surface_fn,
		GraphicsDiagnosticFn on_diagnostic)
		: m_on_diagnostic(std::make_shared<GraphicsDiagnosticFn>(std::move(on_diagnostic)))
	{
		if (m_enable_validation_layers && !validation_layers_are_supported(m_context, m_validation_layers))
		{
			ReportDiagnostic({
				.severity = GraphicsDiagnosticSeverity::Warning,
				.message = "Vulkan validation layers requested, but not available; continuing without validation."
			});
			m_enable_validation_layers = false;
			m_validation_layers.clear();
		}

		auto extension_props = m_context.enumerateInstanceExtensionProperties();
		for (std::uint32_t i = 0; i < extension_count; ++i)
		{
			if (std::ranges::none_of(extension_props,
				[extension = extensions[i]](auto const & extensionProperty)
				{ return strcmp(extensionProperty.extensionName, extension) == 0; }))
			{
				throw GraphicsException("Required Vulkan extension not supported: " + std::string(extensions[i]));
			}
		}

		m_instance = create_instance(m_context, app_title, extension_count, extensions, m_enable_validation_layers, m_validation_layers);

		VkSurfaceKHR raw_surface = create_surface_fn(*m_instance);
		m_surface = vk::raii::SurfaceKHR{ m_instance, raw_surface };

		PhysicalDeviceRequirements const device_requirements{
			.min_api_version = vk::ApiVersion13,
			.required_extensions = m_device_extensions,
			.require_sampler_anisotropy = true,
			.require_dynamic_rendering = true,
			.require_synchronization2 = true,
			.allow_cpu_devices = false
		};
		auto physical_device_result = SelectPhysicalDevice(m_instance, m_surface, device_requirements);
		if (!physical_device_result)
			throw GraphicsException(physical_device_result.error().GetMessage());
		m_phys_device_info = std::move(physical_device_result).value();
		
		m_logical_device = create_logical_device(m_phys_device_info, m_device_extensions);
		m_queue = m_logical_device.getQueue(m_phys_device_info.queue_index, 0);

		m_swap_chain = create_swap_chain(m_phys_device_info, width_pixels, height_pixels,
			m_surface, m_logical_device, m_swap_chain_image_format, m_swap_chain_extent);
		m_swap_chain_images = m_swap_chain.getImages();
		m_swap_chain_image_views = create_swap_chain_image_views(m_logical_device, m_swap_chain_images, m_swap_chain_image_format);

		m_depth_image_format = find_depth_image_format(m_phys_device_info.device);
		create_depth_resources();

		m_command_pool = create_command_pool(m_phys_device_info, m_logical_device);
		m_command_buffers = create_command_buffers(m_logical_device, m_command_pool, MaxFramesInFlight);

		for (size_t i = 0; i < m_swap_chain_images.size(); ++i)
			m_render_finished_semaphores.emplace_back(m_logical_device, vk::SemaphoreCreateInfo{});

		for (size_t i = 0; i < MaxFramesInFlight; ++i)
		{
			m_present_complete_semaphores.emplace_back(m_logical_device, vk::SemaphoreCreateInfo{});
			m_draw_fences.emplace_back(m_logical_device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
		}
	}

	void RenderContext::ReportDiagnostic(GraphicsDiagnostic diagnostic) const noexcept
	{
		if (!m_on_diagnostic || !*m_on_diagnostic)
			return;

		try
		{
			(*m_on_diagnostic)(diagnostic);
		}
		catch (...)
		{
			// Application callbacks must not disrupt renderer control flow.
		}
	}

	void RenderContext::create_depth_resources()
	{
		constexpr std::uint32_t layers = 1;

		m_depth_image = Create2dImage(
			m_swap_chain_extent.width,
			m_swap_chain_extent.height,
			layers,
			m_depth_image_format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::ImageCreateFlags{});

		m_depth_image_memory = CreateImageMemory(m_depth_image, vk::MemoryPropertyFlagBits::eDeviceLocal);

		m_depth_image_view = CreateImageView(
			*m_depth_image,
			vk::ImageViewType::e2D,
			m_depth_image_format,
			vk::ImageAspectFlagBits::eDepth,
			layers);
	}

	void RenderContext::destroy_swap_chain()
	{
		m_depth_image_view.clear();
		m_depth_image_memory.clear();
		m_depth_image.clear();

		m_swap_chain_image_views.clear();
		m_swap_chain_images.clear();
		m_swap_chain.clear();
		m_swap_chain_extent = vk::Extent2D{ 0, 0 };
	}

	bool RenderContext::SwapChainIsValid() const
	{
		return *m_swap_chain != VK_NULL_HANDLE && !m_swap_chain_image_views.empty();
	}

	std::expected<void, GraphicsError> RenderContext::RecreateSwapChain(
		int width_pixels,
		int height_pixels)
	{
		WaitForLastFrame();

		destroy_swap_chain();

		if (width_pixels <= 0 || height_pixels <= 0)
			return {};

		try
		{
			m_phys_device_info.sws_details = QuerySwapChainSupport(m_phys_device_info.device, m_surface);
			vk::Extent2D const extent = choose_swap_extent(
				m_phys_device_info.sws_details.capabilities,
				width_pixels,
				height_pixels);
			if (extent.width == 0 || extent.height == 0)
				return {};

			m_swap_chain = create_swap_chain(m_phys_device_info, width_pixels, height_pixels, m_surface, m_logical_device,
				m_swap_chain_image_format, m_swap_chain_extent);
			m_swap_chain_images = m_swap_chain.getImages();
			m_swap_chain_image_views = create_swap_chain_image_views(m_logical_device, m_swap_chain_images, m_swap_chain_image_format);

			create_depth_resources();
		}
		catch (vk::SystemError const & err)
		{
			return std::unexpected{ GraphicsError{ "Vulkan error: " + std::string(err.what()) } };
		}
		catch (GraphicsException const & err)
		{
			return std::unexpected{ GraphicsError{ "Graphics error: " + std::string(err.what()) } };
		}

		return {};
	}

	DrawFrameResult RenderContext::DrawFrame(std::function<void()> render_fn)
	{
		DrawFrameResult result = DrawFrameResult::Success;

		vk::Result fence_result = m_logical_device.waitForFences(*m_draw_fences[m_current_frame], VK_TRUE, UINT64_MAX);
		if (fence_result != vk::Result::eSuccess)
			throw GraphicsException("Failed to wait for draw fence!");

		try
		{
			auto [ani_result, image_index] = m_swap_chain.acquireNextImage(UINT64_MAX, *m_present_complete_semaphores[m_current_frame], nullptr);
			if (ani_result == vk::Result::eSuboptimalKHR)
				result = DrawFrameResult::SwapChainOutOfDate;
			else if (ani_result != vk::Result::eSuccess)
				throw GraphicsException("Failed to acquire swap chain image!");

			m_current_image_index = image_index;
		}
		catch (vk::OutOfDateKHRError const &) // use VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS when it's available
		{
			return DrawFrameResult::SwapChainOutOfDate;
		}
		catch (vk::SurfaceLostKHRError const &)
		{
			return DrawFrameResult::SurfaceLost;
		}

		// Only reset the fence if we are submitting work
		m_logical_device.resetFences(*m_draw_fences[m_current_frame]);

		m_command_buffers[m_current_frame].reset();
		m_command_buffers[m_current_frame].begin({});

		render_fn();

		vk::PipelineStageFlags wait_stages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo submit_info{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*m_present_complete_semaphores[m_current_frame],
			.pWaitDstStageMask = &wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &*m_command_buffers[m_current_frame],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*m_render_finished_semaphores[m_current_image_index]
		};

		m_queue.submit(submit_info, *m_draw_fences[m_current_frame]);

		vk::PresentInfoKHR present_info{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*m_render_finished_semaphores[m_current_image_index],
			.swapchainCount = 1,
			.pSwapchains = &*m_swap_chain,
			.pImageIndices = &m_current_image_index
		};

		try
		{
			vk::Result present_result = m_queue.presentKHR(present_info);
			if (present_result == vk::Result::eSuboptimalKHR /*|| present_result == vk::Result::eErrorOutOfDateKHR*/)
				result = DrawFrameResult::SwapChainOutOfDate;
		}
		catch (vk::OutOfDateKHRError const &) // use VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS when it's available
		{
			result = DrawFrameResult::SwapChainOutOfDate;
		}
		catch (vk::SurfaceLostKHRError const &)
		{
			result = DrawFrameResult::SurfaceLost;
		}

		m_current_frame = (m_current_frame + 1) % MaxFramesInFlight;
		return result;
	}

	void RenderContext::WaitForLastFrame() const
	{
		m_logical_device.waitIdle();
	}

	std::uint32_t RenderContext::FindMemoryType(std::uint32_t type_filter, vk::MemoryPropertyFlags properties) const
	{
		vk::PhysicalDeviceMemoryProperties const & mem_properties = m_phys_device_info.mem_properties;
		for (std::uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
		{
			if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw GraphicsException("failed to find suitable memory type!");
	}

	vk::raii::Image RenderContext::Create2dImage(
		std::uint32_t width,
		std::uint32_t height,
		std::uint32_t layers,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::ImageCreateFlags flags) const
	{
		vk::ImageCreateInfo image_info{
			.flags = flags,
			.imageType = vk::ImageType::e2D,
			.format = format,
			.extent{
				.width = width,
				.height = height,
				.depth = 1,
			},
			.mipLevels = 1,
			.arrayLayers = layers,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = tiling,
			.usage = usage,
			.sharingMode = vk::SharingMode::eExclusive,
			.initialLayout = vk::ImageLayout::eUndefined,
		};

		return vk::raii::Image{ m_logical_device, image_info };
	}

	vk::raii::DeviceMemory RenderContext::CreateImageMemory(
		vk::raii::Image const & image,
		vk::MemoryPropertyFlags properties) const
	{
		vk::MemoryRequirements mem_requirements = image.getMemoryRequirements();

		std::uint32_t mem_type_index = FindMemoryType(mem_requirements.memoryTypeBits, properties);

		vk::MemoryAllocateInfo alloc_info{
			.allocationSize = mem_requirements.size,
			.memoryTypeIndex = mem_type_index
		};

		// TODO: should use something like VulkanMemoryAllocator library for managing memory
		vk::raii::DeviceMemory image_memory = vk::raii::DeviceMemory{ m_logical_device, alloc_info };

		image.bindMemory(*image_memory, 0);

		return image_memory;
	}

	vk::raii::ImageView RenderContext::CreateImageView(
		vk::Image image,
		vk::ImageViewType view_type,
		vk::Format format,
		vk::ImageAspectFlags aspect_flags,
		std::uint32_t layers) const
	{
		vk::ImageViewCreateInfo create_info{
			.image = image,
			.viewType = view_type,
			.format = format,
			.subresourceRange{
				.aspectMask = aspect_flags,
				.levelCount = 1,
				.layerCount = layers
			}
		};

		return vk::raii::ImageView{ m_logical_device, create_info };
	}

	void RenderContext::DoOneTimeCommand(std::function<void(vk::raii::CommandBuffer const &)> command_fn) const
	{
		vk::raii::CommandBuffers command_buffers = create_command_buffers(m_logical_device, m_command_pool, 1);
		vk::raii::CommandBuffer const & command_buffer = command_buffers[0];

		vk::CommandBufferBeginInfo begin_info{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		};
		command_buffer.begin(begin_info);

		command_fn(command_buffer);

		command_buffer.end();

		vk::SubmitInfo submit_info{
			.commandBufferCount = 1,
			.pCommandBuffers = &*command_buffer
		};

		m_queue.submit(submit_info);
		m_queue.waitIdle();
	}

	void RenderContext::CopyBuffer(
		vk::Buffer src_buffer,
		vk::Buffer dst_buffer,
		vk::DeviceSize size) const
	{
		DoOneTimeCommand([src_buffer, dst_buffer, size](vk::raii::CommandBuffer const & command_buffer)
			{
				vk::BufferCopy copy_region{
					.srcOffset = 0,
					.dstOffset = 0,
					.size = size
				};

				command_buffer.copyBuffer(src_buffer, dst_buffer, copy_region);
			});
	}

	void RenderContext::CopyBufferToImage(vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height, std::uint32_t layers) const
	{
		DoOneTimeCommand([buffer, image, width, height, layers](vk::raii::CommandBuffer const & command_buffer)
			{
				vk::BufferImageCopy region{
					.bufferOffset = 0,
					.bufferRowLength = 0,
					.bufferImageHeight = 0,
					.imageSubresource{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.mipLevel = 0,
						.baseArrayLayer = 0,
						.layerCount = layers,
					},
					.imageOffset{ 0, 0, 0 },
					.imageExtent{ width, height, 1 }
				};

				command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
			});
	}

	void RenderContext::TransitionImageLayout(vk::Image image, std::uint32_t layers, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout) const
	{
		DoOneTimeCommand([image, layers, format, old_layout, new_layout](vk::raii::CommandBuffer const & command_buffer)
			{
				vk::ImageMemoryBarrier barrier{
					.srcAccessMask = vk::AccessFlagBits::eNone,
					.dstAccessMask = vk::AccessFlagBits::eNone,
					.oldLayout = old_layout,
					.newLayout = new_layout,
					.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
					.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
					.image = image,
					.subresourceRange{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.levelCount = 1,
						.layerCount = layers,
					}
				};

				if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
				{
					barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

					if (has_stencil_component(format))
						barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
				}

				vk::PipelineStageFlags src_stage;
				vk::PipelineStageFlags dst_stage;
				if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
				{
					barrier.srcAccessMask = vk::AccessFlagBits::eNone;
					barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

					src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
					dst_stage = vk::PipelineStageFlagBits::eTransfer;
				}
				else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
				{
					barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
					barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

					src_stage = vk::PipelineStageFlagBits::eTransfer;
					dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
				}
				else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
				{
					barrier.srcAccessMask = vk::AccessFlagBits::eNone;
					barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

					src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
					dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
				}
				else
				{
					throw std::invalid_argument("unsupported layout transition!");
				}

				command_buffer.pipelineBarrier(
					src_stage, dst_stage,
					vk::DependencyFlags{},
					{}, {}, barrier);
			});
	}
} // namespace Dreamhearth
