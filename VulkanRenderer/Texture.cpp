// Texture.cpp

module;

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>

#include <vulkan/vulkan.h>

module Texture;

import ImageData;

namespace
{
	VkSampler create_texture_sampler(GraphicsApi const & graphics_api)
	{
		VkPhysicalDeviceProperties const & props = graphics_api.GetPhysicalDeviceInfo().properties;

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

		VkSampler sampler;
		VkResult result = vkCreateSampler(graphics_api.GetDevice(), &sampler_info, nullptr, &sampler);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create vulkan sampler for texture");

		return sampler;
	}

	VkResult load_image_into_buffer(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & filepath,
		VkBuffer & out_buffer,
		VkDeviceMemory & out_buffer_memory,
		VkExtent2D & out_extents)
	{
		ImageData image{ filepath };
		if (!image.IsValid())
		{
			std::cout << "load_image_into_buffer() Failed to load image: " << filepath << std::endl;
			return VK_ERROR_UNKNOWN;
		}

		out_extents = VkExtent2D{
			.width = static_cast<std::uint32_t>(image.GetWidth()),
			.height = static_cast<std::uint32_t>(image.GetHeight())
		};

		VkDeviceSize buffer_size = static_cast<VkDeviceSize>(image.GetSize());

		VkResult result = graphics_api.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			out_buffer,
			out_buffer_memory);
		if (result != VK_SUCCESS)
		{
			std::cout << "load_image_into_buffer() Failed to create staging buffer: " << filepath << std::endl;
			return result;
		}

		VkDevice device = graphics_api.GetDevice();

		void * data = nullptr;
		vkMapMemory(device, out_buffer_memory, 0, buffer_size, 0, &data);
		std::memcpy(data, image.GetData(), image.GetSize());
		vkUnmapMemory(device, out_buffer_memory);

		return VK_SUCCESS; // image goes out of scope and frees memory
	}

	VkResult load_images_into_buffer(
		GraphicsApi const & graphics_api,
		std::array<std::filesystem::path, 6> const & filepaths,
		VkBuffer & out_buffer,
		VkDeviceMemory & out_buffer_memory,
		VkExtent2D & out_extents)
	{
		std::array<ImageData, 6> images;
		for (size_t i = 0; i < filepaths.size(); ++i)
		{
			images[i].LoadImage(filepaths[i]);

			if (!images[i].IsValid())
			{
				std::cout << "load_images_into_buffer() Failed to load image: " << filepaths[i] << std::endl;
				return VK_ERROR_UNKNOWN;
			}
		}

		out_extents = VkExtent2D{
			.width = static_cast<std::uint32_t>(images[0].GetWidth()),
			.height = static_cast<std::uint32_t>(images[0].GetHeight())
		};

		VkDeviceSize buffer_size = 0;
		for (ImageData const & image : images)
			buffer_size += static_cast<VkDeviceSize>(image.GetSize());

		VkResult result = graphics_api.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			out_buffer,
			out_buffer_memory);
		if (result != VK_SUCCESS)
		{
			std::cout << "load_images_into_buffer() Failed to create staging buffer" << std::endl;
			return result;
		}

		VkDevice device = graphics_api.GetDevice();

		void * data = nullptr;
		vkMapMemory(device, out_buffer_memory, 0, buffer_size, 0, &data);
		for (ImageData const & image : images)
		{
			std::memcpy(data, image.GetData(), image.GetSize());
			data = static_cast<unsigned char *>(data) + image.GetSize();
		}
		vkUnmapMemory(device, out_buffer_memory);
		return VK_SUCCESS;
	}
}

Texture::Texture(GraphicsApi const & graphics_api, std::filesystem::path const & filepath)
	: m_graphics_api(graphics_api)
{
	VkDevice device = m_graphics_api.GetDevice();

	VkBuffer staging_buffer = VK_NULL_HANDLE;
	VkDeviceMemory staging_buffer_memory = VK_NULL_HANDLE;
	VkExtent2D extents;
	VkResult result = load_image_into_buffer(graphics_api, filepath, staging_buffer, staging_buffer_memory, extents);
	if (result != VK_SUCCESS)
		return;

	result = graphics_api.Create2dImage(
		extents.width,
		extents.height,
		1 /*layers*/,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		0 /*flags*/,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image,
		m_image_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Texture() Failed to create image: " << filepath << std::endl;
		return;
	}

	m_graphics_api.TransitionImageLayout(
		m_image,
		1 /*layers*/,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_graphics_api.CopyBufferToImage(
		staging_buffer,
		m_image,
		extents.width,
		extents.height,
		1 /*layers*/);
	m_graphics_api.TransitionImageLayout(
		m_image,
		1 /*layers*/,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	result = m_graphics_api.CreateImageView(
		m_image,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		1 /*layers*/,
		m_image_view);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan image view for texture");
		return;
	}

	m_sampler = create_texture_sampler(m_graphics_api);

	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);
}

Texture::Texture(GraphicsApi const & graphics_api, std::array<std::filesystem::path, 6> const & filepaths)
	: m_graphics_api(graphics_api)
{
	VkBuffer staging_buffer = VK_NULL_HANDLE;
	VkDeviceMemory staging_buffer_memory = VK_NULL_HANDLE;
	VkExtent2D extents;
	VkResult result = load_images_into_buffer(graphics_api, filepaths, staging_buffer, staging_buffer_memory, extents);
	if (result != VK_SUCCESS)
		return;

	result = graphics_api.Create2dImage(
		extents.width,
		extents.height,
		6 /*layers*/,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_image,
		m_image_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Texture() Failed to create cubemap: " << filepaths[0] << std::endl;
		return;
	}

	m_graphics_api.TransitionImageLayout(
		m_image,
		6 /*layers*/,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	m_graphics_api.CopyBufferToImage(
		staging_buffer,
		m_image,
		extents.width,
		extents.height,
		6 /*layers*/);

	m_graphics_api.TransitionImageLayout(
		m_image,
		6 /*layers*/,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	result = m_graphics_api.CreateImageView(
		m_image,
		VK_IMAGE_VIEW_TYPE_CUBE,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		6 /*layers*/,
		m_image_view);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan image view for texture");
		return;
	}

	m_sampler = create_texture_sampler(m_graphics_api);

	VkDevice device = m_graphics_api.GetDevice();
	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);
}

Texture::~Texture()
{
	destroy_texture();
}

void Texture::destroy_texture()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroySampler(device, m_sampler, nullptr);
	m_sampler = VK_NULL_HANDLE;
	vkDestroyImageView(device, m_image_view, nullptr);
	m_image_view = VK_NULL_HANDLE;

	vkDestroyImage(device, m_image, nullptr);
	m_image = VK_NULL_HANDLE;
	vkFreeMemory(device, m_image_memory, nullptr);
	m_image_memory = VK_NULL_HANDLE;
}

Texture::Texture(Texture && other)
	: m_graphics_api(other.m_graphics_api)
{
	*this = std::move(other);
}

Texture & Texture::operator=(Texture && other)
{
	if (this == &other)
		return *this;

	destroy_texture();

	m_image = other.m_image;
	m_image_memory = other.m_image_memory;
	m_image_view = other.m_image_view;
	m_sampler = other.m_sampler;

	other.m_image = VK_NULL_HANDLE;
	other.m_image_memory = VK_NULL_HANDLE;
	other.m_image_view = VK_NULL_HANDLE;
	other.m_sampler = VK_NULL_HANDLE;

	return *this;
}

bool Texture::IsValid() const
{
	return m_image != VK_NULL_HANDLE
		&& m_image_memory != VK_NULL_HANDLE
		&& m_image_view != VK_NULL_HANDLE;
}
