// PhysicalDevice.ixx

module;

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

export module Dreamhearth:PhysicalDevice;

import :GraphicsError;

namespace Dreamhearth
{
	export struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> present_modes;
	};

	export constexpr std::uint32_t InvalidQueueIndex = ~0u;

	export struct PhysicalDeviceInfo
	{
		vk::raii::PhysicalDevice device = nullptr;
		std::uint32_t queue_index = InvalidQueueIndex;
		SwapChainSupportDetails sws_details;
		vk::PhysicalDeviceMemoryProperties mem_properties;
		vk::PhysicalDeviceProperties properties;
	};

	export struct PhysicalDeviceRequirements
	{
		std::uint32_t min_api_version = vk::ApiVersion13;
		std::span<char const * const> required_extensions;
		bool require_sampler_anisotropy = true;
		bool require_dynamic_rendering = true;
		bool require_synchronization2 = true;
		bool allow_cpu_devices = false;
	};

	export struct PhysicalDeviceEvaluation
	{
		PhysicalDeviceInfo info;
		std::vector<std::string> rejection_reasons;

		bool IsSuitable() const { return rejection_reasons.empty(); }
	};

	export SwapChainSupportDetails QuerySwapChainSupport(
		vk::raii::PhysicalDevice const & device,
		vk::raii::SurfaceKHR const & surface);

	export PhysicalDeviceEvaluation EvaluatePhysicalDevice(
		vk::raii::PhysicalDevice const & device,
		vk::raii::SurfaceKHR const & surface,
		PhysicalDeviceRequirements const & requirements);

	export std::string FormatPhysicalDeviceSelectionError(
		std::span<PhysicalDeviceEvaluation const> evaluations);

	export std::expected<PhysicalDeviceInfo, GraphicsError> SelectPhysicalDevice(
		vk::raii::Instance const & instance,
		vk::raii::SurfaceKHR const & surface,
		PhysicalDeviceRequirements const & requirements);
} // namespace Dreamhearth
