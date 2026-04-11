// Texture.ixx

module;

#include <array>
#include <cstdint>
#include <expected>

export module Texture;

import GraphicsApi;
import GraphicsError;

export enum class PixelFormat : std::uint8_t { RGB_UNORM, RGBA_UNORM, RGB_SRGB, RGBA_SRGB };

export std::uint8_t GetPixelSize(PixelFormat format)
{
	if (format == PixelFormat::RGBA_UNORM || format == PixelFormat::RGBA_SRGB)
		return 4;
	if (format == PixelFormat::RGB_UNORM || format == PixelFormat::RGB_SRGB)
		return 3;
	return 0;
}

export struct ImageData
{
	std::uint8_t const * data = nullptr;
	PixelFormat format = PixelFormat::RGBA_SRGB;
	std::uint32_t width = 0;
	std::uint32_t height = 0;

	bool IsValid() const;
	std::uint64_t GetSize() const;
};

export struct CubeImageData
{
	std::array<std::uint8_t const *, 6> data;
	PixelFormat format = PixelFormat::RGBA_SRGB;
	std::uint32_t width = 0;
	std::uint32_t height = 0;

	bool IsValid() const;
	std::uint64_t GetSize() const;
};

class Image
{
public:
	Image() = default;
	~Image();

	Image(Image && other);
	Image & operator=(Image && other);

	Image(Image const &) = delete;
	Image & operator=(Image const &) = delete;

	void Create();

	unsigned int GetId() const { return m_id; }

private:
	unsigned int m_id = 0;
};

export class Texture
{
public:
	Texture() = default;

	Texture(Texture && other) = default;
	Texture & operator=(Texture && other) = default;

	Texture(Texture const &) = delete;
	Texture & operator=(Texture const &) = delete;

	std::expected<void, GraphicsError> Create(GraphicsApi const & graphics_api, ImageData const & image_data, bool use_mip_map = true);
	std::expected<void, GraphicsError> Create(GraphicsApi const & graphics_api, CubeImageData const & image_data);

	bool IsValid() const;

	unsigned int GetId() const { return m_image.GetId(); }
	unsigned int GetType() const { return m_type; }

	std::uint32_t GetWidth() const { return m_width; }
	std::uint32_t GetHeight() const { return m_height; }

	static void Bind(unsigned int id, unsigned int type, unsigned int binding);

private:
	unsigned int m_type = 0;
	Image m_image;
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;
};
