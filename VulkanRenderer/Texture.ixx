// Texture.ixx

module;

#include <array>
#include <filesystem>

#include <vulkan/vulkan.h>

export module Texture;

import GraphicsApi;

export enum class PixelFormat : std::uint8_t { RGB_UNORM, RGBA_UNORM, RGB_SRGB, RGBA_SRGB };

export std::uint8_t GetPixelSize(PixelFormat format)
{
	if (format == PixelFormat::RGBA_UNORM || format == PixelFormat::RGBA_SRGB)
		return 4;
	if (format == PixelFormat::RGB_UNORM || format == PixelFormat::RGB_SRGB)
		return 3;
	throw std::runtime_error("GetPixelSize: Unexpected format");
	return 0;
}

export class Texture
{
public:
	struct ImageData
	{
		std::uint8_t const * m_data = nullptr;
		PixelFormat m_format = PixelFormat::RGBA_SRGB;
		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;

		bool IsValid() const;
		std::uint64_t GetSize() const;
	};

	struct CubeImageData
	{
		std::array<std::uint8_t const *, 6> m_data;
		PixelFormat m_format = PixelFormat::RGBA_SRGB;
		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;

		bool IsValid() const;
		std::uint64_t GetSize() const;
	};

public:
	Texture(GraphicsApi const & graphics_api, ImageData const & image_data, bool use_mip_map = true);
	Texture(GraphicsApi const & graphics_api, CubeImageData const & image_data);
	~Texture();

	Texture(Texture && other);
	Texture & operator=(Texture && other);

	Texture(Texture &) = delete;
	Texture & operator=(Texture &) = delete;

	bool IsValid() const;

	VkImageView GetImageView() const { return m_image_view; }
	VkSampler GetSampler() const { return m_sampler; }

private:
	void destroy_texture();

private:
	GraphicsApi const & m_graphics_api;

	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_image_memory = VK_NULL_HANDLE;
	VkImageView m_image_view = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;
};
