// Texture.cpp

module;

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <expected>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

module Texture;

import Buffer;
import GraphicsError;

vk::Format to_vk_format(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::RGBA_UNORM:
		return vk::Format::eR8G8B8A8Unorm;
	case PixelFormat::RGB_UNORM:
		return vk::Format::eR8G8B8A8Unorm; //vk::Format::eR8G8B8Unorm;
	case PixelFormat::RGBA_SRGB:
		return vk::Format::eR8G8B8A8Srgb;
	case PixelFormat::RGB_SRGB:
		return vk::Format::eR8G8B8Srgb;
	default:
		return vk::Format::eUndefined;
	}
}

bool ImageData::IsValid() const
{
	return GetSize() > 0 && data != nullptr;
}

std::uint64_t ImageData::GetSize() const
{
	return static_cast<std::uint64_t>(width * height * GetPixelSize(format));
}

bool CubeImageData::IsValid() const
{
	return GetSize() > 0
		&& std::ranges::all_of(data, [](std::uint8_t  const * data) { return data != nullptr; });
}

std::uint64_t CubeImageData::GetSize() const
{
	return static_cast<std::uint64_t>(width * height * GetPixelSize(format));
}

Buffer load_image_into_buffer(
	GraphicsApi const & graphics_api,
	ImageData const & image_data)
{
	std::uint64_t input_size = image_data.GetSize();

	const void * data_ptr = image_data.data;
	std::uint64_t buffer_size = input_size;

	std::vector<std::uint8_t> rgba_data;
	if (image_data.format == PixelFormat::RGB_UNORM)
	{
		// Convert RGB to RGBA
		const std::uint8_t * src = image_data.data;
		const std::size_t pixel_count = image_data.width * image_data.height;
		rgba_data.reserve(pixel_count * 4);
		for (std::size_t i = 0; i < pixel_count; ++i) {
			rgba_data.push_back(src[i * 3 + 0]);
			rgba_data.push_back(src[i * 3 + 1]);
			rgba_data.push_back(src[i * 3 + 2]);
			rgba_data.push_back(255); // Opaque alpha
		}
		data_ptr = rgba_data.data();
		buffer_size = pixel_count * 4;
	}

	Buffer out_buffer;
	out_buffer.Create(
		graphics_api,
		buffer_size,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void * buffer_data = out_buffer.GetMemory().mapMemory(0, buffer_size);
	std::memcpy(buffer_data, data_ptr, buffer_size);
	out_buffer.GetMemory().unmapMemory();

	return out_buffer;
}

Buffer load_cube_image_into_buffer(
	GraphicsApi const & graphics_api,
	CubeImageData const & image_data)
{
	std::uint64_t image_size = image_data.GetSize();
	std::uint64_t buffer_size = image_size * image_data.data.size();

	Buffer out_buffer;
	out_buffer.Create(
		graphics_api,
		buffer_size,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	VkDevice device = *graphics_api.GetDevice();

	void * buffer_data = out_buffer.GetMemory().mapMemory(0, buffer_size);
	for (std::uint8_t const * data : image_data.data)
	{
		std::memcpy(buffer_data, data, image_size);
		buffer_data = static_cast<std::uint8_t *>(buffer_data) + image_size;
	}
	out_buffer.GetMemory().unmapMemory();

	return out_buffer;
}

vk::raii::Sampler create_sampler(GraphicsApi const & graphics_api)
{
	vk::PhysicalDeviceProperties const & props = graphics_api.GetPhysicalDeviceInfo().properties;

	vk::SamplerCreateInfo sampler_info{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = vk::True,
		.maxAnisotropy = props.limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = vk::BorderColor::eIntOpaqueBlack,
		.unnormalizedCoordinates = vk::False
	};

	return vk::raii::Sampler{ graphics_api.GetDevice(), sampler_info };
}

std::expected<void, GraphicsError> Texture::Create(GraphicsApi const & graphics_api, ImageData const & image_data, bool use_mip_map /*= true*/)
{
	if (!image_data.IsValid())
		return std::unexpected{ GraphicsError{ "create_2d_image: image_data not valid" } };

	vk::Format format = to_vk_format(image_data.format);
	if (format == vk::Format::eUndefined)
		return std::unexpected{ GraphicsError{ "create_2d_image: Unsupported pixel format: " + std::to_string(static_cast<int>(format)) } };

	try
	{
		Buffer staging_buffer = load_image_into_buffer(graphics_api, image_data);

		constexpr std::uint32_t layers = 1;

		m_image = graphics_api.Create2dImage(
			image_data.width,
			image_data.height,
			layers,
			format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::ImageCreateFlags{});

		m_image_memory = graphics_api.CreateImageMemory(m_image, vk::MemoryPropertyFlagBits::eDeviceLocal);

		graphics_api.TransitionImageLayout(*m_image, layers, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		graphics_api.CopyBufferToImage(*staging_buffer.Get(), *m_image, image_data.width, image_data.height, layers);
		graphics_api.TransitionImageLayout(*m_image, layers, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		m_image_view = graphics_api.CreateImageView(
			*m_image,
			vk::ImageViewType::e2D,
			format,
			vk::ImageAspectFlagBits::eColor,
			layers);

		m_sampler = create_sampler(graphics_api);
	}
	catch (vk::SystemError & err)
	{
		return std::unexpected{ GraphicsError{ "Texture::Create: Failed to create texture. " + std::string(err.what()) } };
	}

	m_width = image_data.width;
	m_height = image_data.height;

	return {};
}

std::expected<void, GraphicsError> Texture::Create(GraphicsApi const & graphics_api, CubeImageData const & image_data)
{
	if (!image_data.IsValid())
		return std::unexpected{ GraphicsError{ "create_cube_image: image_data not valid" } };

	vk::Format format = to_vk_format(image_data.format);
	if (format == vk::Format::eUndefined)
		return std::unexpected{ GraphicsError{ "create_cube_image: Unsupported pixel format: " + std::to_string(static_cast<int>(format)) } };

	try
	{
		Buffer staging_buffer = load_cube_image_into_buffer(graphics_api, image_data);

		constexpr std::uint32_t layers = 6;

		m_image = graphics_api.Create2dImage(
			image_data.width,
			image_data.height,
			layers,
			format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::ImageCreateFlagBits::eCubeCompatible);

		m_image_memory = graphics_api.CreateImageMemory(m_image, vk::MemoryPropertyFlagBits::eDeviceLocal);

		graphics_api.TransitionImageLayout(*m_image, layers, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		graphics_api.CopyBufferToImage(*staging_buffer.Get(), *m_image, image_data.width, image_data.height, layers);
		graphics_api.TransitionImageLayout(*m_image, layers, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		m_image_view = graphics_api.CreateImageView(
			*m_image,
			vk::ImageViewType::eCube,
			format,
			vk::ImageAspectFlagBits::eColor,
			layers);

		m_sampler = create_sampler(graphics_api);
	}
	catch (vk::SystemError & err)
	{
		return std::unexpected{ GraphicsError{ "Texture::Create: Failed to create cube texture. " + std::string(err.what()) } };
	}

	m_width = image_data.width;
	m_height = image_data.height;

	return {};
}

bool Texture::IsValid() const
{
	return m_image != nullptr
		&& m_image_memory != nullptr
		&& m_image_view != nullptr
		&& m_sampler != nullptr
		&& m_width != 0
		&& m_height != 0;
}
