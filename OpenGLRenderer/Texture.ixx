// Texture.ixx

module;

#include <array>
#include <filesystem>

export module Texture;

export enum class PixelFormat : std::uint8_t { RGB, RGBA };

export std::uint8_t GetPixelSize(PixelFormat format)
{
	if (format == PixelFormat::RGBA)
		return 4;
	if (format == PixelFormat::RGB)
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
		PixelFormat m_format = PixelFormat::RGBA;
		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;

		bool IsValid() const;
		std::uint64_t GetSize() const;
	};

	struct CubeImageData
	{
		std::array<std::uint8_t const *, 6> m_data;
		PixelFormat m_format = PixelFormat::RGBA;
		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;

		bool IsValid() const;
		std::uint64_t GetSize() const;
	};

public:
	Texture(ImageData const & image_data, bool use_mip_map = true);
	Texture(CubeImageData const & image_data);
	~Texture();

	Texture(Texture && other);
	Texture & operator=(Texture && other);

	Texture(Texture &) = delete;
	Texture & operator=(Texture &) = delete;

	bool IsValid() const;

	void Bind() const;

private:
	void destroy_texture();

private:
	unsigned int m_type = 0;
	unsigned int m_tex_id = 0;
};
