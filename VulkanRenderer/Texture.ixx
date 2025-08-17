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

export struct ImageData
{
	std::uint8_t const * m_data = nullptr;
	PixelFormat m_format = PixelFormat::RGBA_SRGB;
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;

	bool IsValid() const;
	std::uint64_t GetSize() const;
};

export struct CubeImageData
{
	std::array<std::uint8_t const *, 6> m_data;
	PixelFormat m_format = PixelFormat::RGBA_SRGB;
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;

	bool IsValid() const;
	std::uint64_t GetSize() const;
};

class Image
{
public:
	Image(GraphicsApi const & graphics_api) : m_graphics_api(graphics_api) {}
	~Image();

	Image(Image && other);
	Image & operator=(Image && other);

	Image(Image const &) = delete;
	Image & operator=(Image const &) = delete;

	VkResult Create2dImage(ImageData const & image_data);
	VkResult CreateCubeImage(CubeImageData const & image_data);

	VkImage Get() const { return m_image; }
	VkDeviceMemory GetMemory() const { return m_image_memory; }
	VkImageView GetView() const { return m_image_view; }

private:
	void destroy();

private:
	GraphicsApi const & m_graphics_api;

	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_image_memory = VK_NULL_HANDLE;
	VkImageView m_image_view = VK_NULL_HANDLE;
};

class Sampler
{
public:
	Sampler(GraphicsApi const & graphics_api) : m_graphics_api(graphics_api) {}
	~Sampler();

	Sampler(Sampler && other);
	Sampler & operator=(Sampler && other);

	Sampler(Sampler const &) = delete;
	Sampler & operator=(Sampler const &) = delete;

	VkResult Create();

	VkSampler Get() const { return m_sampler; }

private:
	void destroy();

private:
	GraphicsApi const & m_graphics_api;

	VkSampler m_sampler = VK_NULL_HANDLE;
};

export class Texture
{
public:
	Texture(GraphicsApi const & graphics_api, ImageData const & image_data, bool use_mip_map = true);
	Texture(GraphicsApi const & graphics_api, CubeImageData const & image_data);
	~Texture() = default;

	Texture(Texture && other) = default;
	Texture & operator=(Texture && other) = default;

	Texture(Texture const &) = delete;
	Texture & operator=(Texture const &) = delete;

	bool IsValid() const;

	VkImageView GetImageView() const { return m_image.GetView(); }
	VkSampler GetSampler() const { return m_sampler.Get(); }

private:
	std::reference_wrapper<GraphicsApi const> m_graphics_api;

	Image m_image;
	Sampler m_sampler;
};
