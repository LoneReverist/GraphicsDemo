// PhysicalDevice.cpp

module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <limits>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

module Dreamhearth;

import :PhysicalDevice;
import :GraphicsError;

namespace Dreamhearth
{
	namespace
	{
		std::string FormatApiVersion(std::uint32_t version)
		{
			return std::to_string(VK_API_VERSION_MAJOR(version)) + "."
				+ std::to_string(VK_API_VERSION_MINOR(version)) + "."
				+ std::to_string(VK_API_VERSION_PATCH(version));
		}

		std::string_view DeviceTypeName(vk::PhysicalDeviceType type)
		{
			switch (type)
			{
			case vk::PhysicalDeviceType::eDiscreteGpu:
				return "discrete GPU";
			case vk::PhysicalDeviceType::eIntegratedGpu:
				return "integrated GPU";
			case vk::PhysicalDeviceType::eVirtualGpu:
				return "virtual GPU";
			case vk::PhysicalDeviceType::eCpu:
				return "CPU";
			default:
				return "other device";
			}
		}

		int DeviceTypeRank(vk::PhysicalDeviceType type)
		{
			switch (type)
			{
			case vk::PhysicalDeviceType::eDiscreteGpu:
				return 0;
			case vk::PhysicalDeviceType::eIntegratedGpu:
				return 1;
			case vk::PhysicalDeviceType::eVirtualGpu:
				return 2;
			case vk::PhysicalDeviceType::eCpu:
				return 4;
			default:
				return 3;
			}
		}

		std::uint32_t FindQueueFamily(
			vk::raii::PhysicalDevice const & device,
			vk::raii::SurfaceKHR const & surface)
		{
			auto queue_families = device.getQueueFamilyProperties();
			for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queue_families.size()); ++i)
			{
				if (queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics
					&& device.getSurfaceSupportKHR(i, surface))
					return i;
			}
			return InvalidQueueIndex;
		}
	}

	SwapChainSupportDetails QuerySwapChainSupport(
		vk::raii::PhysicalDevice const & device,
		vk::raii::SurfaceKHR const & surface)
	{
		return SwapChainSupportDetails{
			.capabilities = device.getSurfaceCapabilitiesKHR(*surface),
			.formats = device.getSurfaceFormatsKHR(*surface),
			.present_modes = device.getSurfacePresentModesKHR(*surface)
		};
	}

	PhysicalDeviceEvaluation EvaluatePhysicalDevice(
		vk::raii::PhysicalDevice const & device,
		vk::raii::SurfaceKHR const & surface,
		PhysicalDeviceRequirements const & requirements)
	{
		PhysicalDeviceEvaluation evaluation;
		evaluation.info.device = device;
		evaluation.info.properties = device.getProperties();
		evaluation.info.mem_properties = device.getMemoryProperties();

		vk::PhysicalDeviceProperties const & properties = evaluation.info.properties;
		if (properties.apiVersion < requirements.min_api_version)
		{
			evaluation.rejection_reasons.push_back(
				"Supports Vulkan " + FormatApiVersion(properties.apiVersion)
				+ "; Vulkan " + FormatApiVersion(requirements.min_api_version) + " or newer is required.");
		}

		if (properties.deviceType == vk::PhysicalDeviceType::eCpu && !requirements.allow_cpu_devices)
			evaluation.rejection_reasons.emplace_back("Software Vulkan devices are not supported.");

		try
		{
			evaluation.info.queue_index = FindQueueFamily(device, surface);
			if (evaluation.info.queue_index == InvalidQueueIndex)
				evaluation.rejection_reasons.emplace_back("No queue family supports both graphics and presentation.");
		}
		catch (vk::SystemError const & err)
		{
			evaluation.rejection_reasons.emplace_back(
				"Failed to query graphics and presentation queue support: " + std::string{ err.what() } + ".");
		}

		try
		{
			auto available_extensions = device.enumerateDeviceExtensionProperties();
			for (char const * required_extension : requirements.required_extensions)
			{
				bool const supported = std::ranges::any_of(available_extensions,
					[required_extension](vk::ExtensionProperties const & available_extension)
					{
						return std::string_view{ available_extension.extensionName } == required_extension;
					});
				if (!supported)
					evaluation.rejection_reasons.emplace_back(
						"Missing required device extension: " + std::string{ required_extension } + ".");
			}
		}
		catch (vk::SystemError const & err)
		{
			evaluation.rejection_reasons.emplace_back(
				"Failed to enumerate device extensions: " + std::string{ err.what() } + ".");
		}

		try
		{
			evaluation.info.sws_details = QuerySwapChainSupport(device, surface);
			if (evaluation.info.sws_details.formats.empty())
				evaluation.rejection_reasons.emplace_back("No compatible swapchain surface formats are available.");
			if (evaluation.info.sws_details.present_modes.empty())
				evaluation.rejection_reasons.emplace_back("No compatible swapchain presentation modes are available.");
		}
		catch (vk::SystemError const & err)
		{
			evaluation.rejection_reasons.emplace_back(
				"Failed to query swapchain support: " + std::string{ err.what() } + ".");
		}

		try
		{
			vk::PhysicalDeviceFeatures const features = device.getFeatures();
			if (requirements.require_sampler_anisotropy && !features.samplerAnisotropy)
				evaluation.rejection_reasons.emplace_back("Sampler anisotropy is unavailable.");
		}
		catch (vk::SystemError const & err)
		{
			evaluation.rejection_reasons.emplace_back(
				"Failed to query physical-device features: " + std::string{ err.what() } + ".");
		}

		// Vulkan 1.3 feature structures are only queried from devices that report Vulkan 1.3.
		// Older devices are already rejected by the version requirement, and querying newer
		// structures from an older implementation is not a reliable way to diagnose them.
		if (properties.apiVersion >= vk::ApiVersion13)
		{
			try
			{
				auto features2 = device.template getFeatures2<
					vk::PhysicalDeviceFeatures2,
					vk::PhysicalDeviceVulkan13Features>();
				vk::PhysicalDeviceVulkan13Features const & features13 =
					features2.template get<vk::PhysicalDeviceVulkan13Features>();

				if (requirements.require_dynamic_rendering && !features13.dynamicRendering)
					evaluation.rejection_reasons.emplace_back("Dynamic rendering is unavailable.");
				if (requirements.require_synchronization2 && !features13.synchronization2)
					evaluation.rejection_reasons.emplace_back("Synchronization2 is unavailable.");
			}
			catch (vk::SystemError const & err)
			{
				evaluation.rejection_reasons.emplace_back(
					"Failed to query Vulkan 1.3 features: " + std::string{ err.what() } + ".");
			}
		}

		return evaluation;
	}

	std::string FormatPhysicalDeviceSelectionError(
		std::span<PhysicalDeviceEvaluation const> evaluations)
	{
		if (evaluations.empty())
			return "No Vulkan physical devices were found.";

		std::string message = "No physical device meets the Vulkan renderer requirements.\n\nDevices evaluated:";
		for (PhysicalDeviceEvaluation const & evaluation : evaluations)
		{
			vk::PhysicalDeviceProperties const & properties = evaluation.info.properties;
			message += "\n\n";
			message += properties.deviceName[0] != '\0'
				? properties.deviceName.data()
				: "Unknown Vulkan device";
			message += " (";
			message += DeviceTypeName(properties.deviceType);
			message += ", Vulkan ";
			message += FormatApiVersion(properties.apiVersion);
			message += "):";

			for (std::string const & reason : evaluation.rejection_reasons)
				message += "\n  - " + reason;
		}
		return message;
	}

	std::expected<PhysicalDeviceInfo, GraphicsError> SelectPhysicalDevice(
		vk::raii::Instance const & instance,
		vk::raii::SurfaceKHR const & surface,
		PhysicalDeviceRequirements const & requirements)
	{
		auto devices = instance.enumeratePhysicalDevices();
		std::vector<PhysicalDeviceEvaluation> evaluations;
		evaluations.reserve(devices.size());
		for (vk::raii::PhysicalDevice const & device : devices)
		{
			try
			{
				evaluations.push_back(EvaluatePhysicalDevice(device, surface, requirements));
			}
			catch (vk::SystemError const & err)
			{
				PhysicalDeviceEvaluation evaluation;
				evaluation.info.device = device;
				try
				{
					evaluation.info.properties = device.getProperties();
				}
				catch (vk::SystemError const &)
				{
					// The formatter will identify this adapter as an unknown Vulkan device.
				}
				evaluation.rejection_reasons.emplace_back(
					"Failed to inspect this physical device: " + std::string{ err.what() } + ".");
				evaluations.push_back(std::move(evaluation));
			}
		}

		auto best_device = std::ranges::min_element(evaluations,
			[](PhysicalDeviceEvaluation const & lhs, PhysicalDeviceEvaluation const & rhs)
			{
				int const lhs_rank = lhs.IsSuitable()
					? DeviceTypeRank(lhs.info.properties.deviceType)
					: std::numeric_limits<int>::max();
				int const rhs_rank = rhs.IsSuitable()
					? DeviceTypeRank(rhs.info.properties.deviceType)
					: std::numeric_limits<int>::max();
				return lhs_rank < rhs_rank;
			});

		if (best_device == evaluations.end() || !best_device->IsSuitable())
			return std::unexpected{ GraphicsError{ FormatPhysicalDeviceSelectionError(evaluations) } };

		return best_device->info;
	}
} // namespace Dreamhearth
