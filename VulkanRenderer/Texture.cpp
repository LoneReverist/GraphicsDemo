// Texture.cpp

module;

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <expected>
#include <vector>

#include <vulkan/vulkan.h>

module Texture;

import Buffer;
import GraphicsError;

VkFormat to_vk_format(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::RGBA_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case PixelFormat::RGB_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM; //VK_FORMAT_R8G8B8_UNORM;
	case PixelFormat::RGBA_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case PixelFormat::RGB_SRGB:
		return VK_FORMAT_R8G8B8_SRGB;
	default:
		return VK_FORMAT_UNDEFINED;
	}
}

bool ImageData::IsValid() const
{
	return GetSize() > 0 && m_data != nullptr;
}

std::uint64_t ImageData::GetSize() const
{
	return static_cast<std::uint64_t>(m_width * m_height * GetPixelSize(m_format));
}

bool CubeImageData::IsValid() const
{
	return GetSize() > 0
		&& std::ranges::all_of(m_data, [](std::uint8_t  const * data) { return data != nullptr; });
}

std::uint64_t CubeImageData::GetSize() const
{
	return static_cast<std::uint64_t>(m_width * m_height * GetPixelSize(m_format));
}

Image::~Image()
{
	destroy();
}

Image::Image(Image && other)
	: m_graphics_api(other.m_graphics_api)
{
	*this = std::move(other);
}

Image & Image::operator=(Image && other)
{
	if (this != &other)
	{
		destroy();

		std::swap(m_image, other.m_image);
		std::swap(m_image_memory, other.m_image_memory);
		std::swap(m_image_view, other.m_image_view);
	}
	return *this;
}

void Image::destroy()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroyImageView(device, m_image_view, nullptr);
	m_image_view = VK_NULL_HANDLE;
	vkDestroyImage(device, m_image, nullptr);
	m_image = VK_NULL_HANDLE;
	vkFreeMemory(device, m_image_memory, nullptr);
	m_image_memory = VK_NULL_HANDLE;
}

VkResult load_image_into_buffer(
	GraphicsApi const & graphics_api,
	ImageData const & image_data,
	Buffer & out_buffer)
{
	std::uint64_t input_size = image_data.GetSize();

	const void * data_ptr = image_data.m_data;
	std::uint64_t buffer_size = input_size;

	std::vector<std::uint8_t> rgba_data;
	if (image_data.m_format == PixelFormat::RGB_UNORM)
	{
		// Convert RGB to RGBA
		const std::uint8_t * src = image_data.m_data;
		const std::size_t pixel_count = image_data.m_width * image_data.m_height;
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

	VkResult result = out_buffer.Create(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (result != VK_SUCCESS)
		return result;

	VkDevice device = graphics_api.GetDevice();

	void * buffer_data = nullptr;
	vkMapMemory(device, out_buffer.GetMemory(), 0, buffer_size, 0, &buffer_data);
	std::memcpy(buffer_data, data_ptr, buffer_size);
	vkUnmapMemory(device, out_buffer.GetMemory());

	return VK_SUCCESS;
}

std::expected<void, GraphicsError> Image::Create2dImage(ImageData const & image_data)
{
	if (!image_data.IsValid())
		return std::unexpected{ GraphicsError{ "Image::Create2dImage image_data not valid" } };

	VkDevice device = m_graphics_api.GetDevice();

	Buffer staging_buffer{ m_graphics_api };
	VkResult result = load_image_into_buffer(m_graphics_api, image_data, staging_buffer);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Image::Create2dImage Failed to create staging buffer" } };

	VkFormat format = to_vk_format(image_data.m_format);
	if (format == VK_FORMAT_UNDEFINED)
		return std::unexpected{ GraphicsError{ "Image::Create2dImage Unsupported pixel format" } };;

	result = m_graphics_api.Create2dImage(
		image_data.m_width,
		image_data.m_height,
		1 /*layers*/,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		0 /*flags*/,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image,
		m_image_memory);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Image::Create2dImage Failed to create image" } };

	m_graphics_api.TransitionImageLayout(
		m_image,
		1 /*layers*/,
		format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_graphics_api.CopyBufferToImage(
		staging_buffer.Get(),
		m_image,
		image_data.m_width,
		image_data.m_height,
		1 /*layers*/);
	m_graphics_api.TransitionImageLayout(
		m_image,
		1 /*layers*/,
		format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	result = m_graphics_api.CreateImageView(
		m_image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		VK_IMAGE_ASPECT_COLOR_BIT,
		1 /*layers*/,
		m_image_view);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Image::Create2dImage Failed to create vulkan image view for texture" } };

	return {};
}

VkResult load_cube_image_into_buffer(
	GraphicsApi const & graphics_api,
	CubeImageData const & image_data,
	Buffer & out_buffer)
{
	std::uint64_t image_size = image_data.GetSize();
	std::uint64_t buffer_size = image_size * image_data.m_data.size();

	VkResult result = out_buffer.Create(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (result != VK_SUCCESS)
		return result;

	VkDevice device = graphics_api.GetDevice();

	void * buffer_data = nullptr;
	vkMapMemory(device, out_buffer.GetMemory(), 0, buffer_size, 0, &buffer_data);
	for (std::uint8_t const * data : image_data.m_data)
	{
		std::memcpy(buffer_data, data, image_size);
		buffer_data = static_cast<std::uint8_t *>(buffer_data) + image_size;
	}
	vkUnmapMemory(device, out_buffer.GetMemory());
	return VK_SUCCESS;
}

std::expected<void, GraphicsError> Image::CreateCubeImage(CubeImageData const & image_data)
{
	if (!image_data.IsValid())
		return std::unexpected{ GraphicsError{ "Image::CreateCubeImage image_data not valid" } };

	VkDevice device = m_graphics_api.GetDevice();

	Buffer staging_buffer{ m_graphics_api };
	VkResult result = load_cube_image_into_buffer(m_graphics_api, image_data, staging_buffer);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Image::CreateCubeImage Failed to create staging buffer" } };

	VkFormat format = to_vk_format(image_data.m_format);
	if (format == VK_FORMAT_UNDEFINED)
		return std::unexpected{ GraphicsError{ "Image::Create2dImage Unsupported pixel format" } };

	result = m_graphics_api.Create2dImage(
		image_data.m_width,
		image_data.m_height,
		6 /*layers*/,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image,
		m_image_memory);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Image::CreateCubeImage Failed to create cubemap" } };

	m_graphics_api.TransitionImageLayout(
		m_image,
		6 /*layers*/,
		format,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	m_graphics_api.CopyBufferToImage(
		staging_buffer.Get(),
		m_image,
		image_data.m_width,
		image_data.m_height,
		6 /*layers*/);

	m_graphics_api.TransitionImageLayout(
		m_image,
		6 /*layers*/,
		format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	result = m_graphics_api.CreateImageView(
		m_image,
		VK_IMAGE_VIEW_TYPE_CUBE,
		format,
		VK_IMAGE_ASPECT_COLOR_BIT,
		6 /*layers*/,
		m_image_view);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Image::CreateCubeImage Failed to create vulkan image view for texture" } };

	return {};
}

Sampler::~Sampler()
{
	destroy();
}

Sampler::Sampler(Sampler && other)
	: m_graphics_api(other.m_graphics_api)
{
	*this = std::move(other);
}

Sampler & Sampler::operator=(Sampler && other)
{
	if (this != &other)
	{
		destroy();

		std::swap(m_sampler, other.m_sampler);
	}
	return *this;
}

void Sampler::destroy()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroySampler(device, m_sampler, nullptr);
	m_sampler = VK_NULL_HANDLE;
}

std::expected<void, GraphicsError> Sampler::Create()
{
	VkPhysicalDeviceProperties const & props = m_graphics_api.GetPhysicalDeviceInfo().properties;

	VkSamplerCreateInfo sampler_info{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = props.limits.maxSamplerAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};

	VkResult result = vkCreateSampler(m_graphics_api.GetDevice(), &sampler_info, nullptr, &m_sampler);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "Failed to create vulkan sampler for texture" } };

	return {};
}

Texture::Texture(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
	, m_image(graphics_api)
	, m_sampler(graphics_api)
{
}

std::expected<void, GraphicsError> Texture::Create(ImageData const & image_data, bool use_mip_map /*= true*/)
{
	std::expected<void, GraphicsError> image_result = m_image.Create2dImage(image_data);
	if (!image_result.has_value())
		return image_result;

	std::expected<void, GraphicsError> sampler_result = m_sampler.Create();
	if (!sampler_result.has_value())
		return sampler_result;

	return {};
}

std::expected<void, GraphicsError> Texture::Create(CubeImageData const & image_data)
{
	std::expected<void, GraphicsError> image_result = m_image.CreateCubeImage(image_data);
	if (!image_result.has_value())
		return image_result;

	std::expected<void, GraphicsError> sampler_result = m_sampler.Create();
	if (!sampler_result.has_value())
		return sampler_result;

	return {};
}

bool Texture::IsValid() const
{
	return m_image.Get() != VK_NULL_HANDLE
		&& m_image.GetMemory() != VK_NULL_HANDLE
		&& m_image.GetView() != VK_NULL_HANDLE
		&& m_sampler.Get() != VK_NULL_HANDLE;
}
