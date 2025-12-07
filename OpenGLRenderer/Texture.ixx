// Texture.ixx

module;

#include <array>
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
	explicit Texture(GraphicsApi const & graphics_api);
	~Texture() = default;

	Texture(Texture && other) = default;
	Texture & operator=(Texture && other) = default;

	Texture(Texture const &) = delete;
	Texture & operator=(Texture const &) = delete;

	std::expected<void, GraphicsError> Create(ImageData const & image_data, bool use_mip_map = true);
	std::expected<void, GraphicsError> Create(CubeImageData const & image_data);

	bool IsValid() const;

	void Bind(unsigned int binding) const;

private:
	std::reference_wrapper<GraphicsApi const> m_graphics_api;

	unsigned int m_type = 0;
	Image m_image;
};
