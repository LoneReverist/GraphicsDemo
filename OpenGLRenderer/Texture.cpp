// Texture.cpp

module;

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <string>

#include <glad/glad.h>

module Texture;

GLenum to_gl_internal_format(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::RGBA_UNORM:
		return GL_RGBA8;
	case PixelFormat::RGB_UNORM:
		return GL_RGB8;
	case PixelFormat::RGBA_SRGB:
		return GL_SRGB8_ALPHA8;
	case PixelFormat::RGB_SRGB:
		return GL_SRGB8;
	default:
		return 0;
	}
}

GLenum to_gl_format(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::RGBA_UNORM:
	case PixelFormat::RGBA_SRGB:
		return GL_RGBA;
	case PixelFormat::RGB_UNORM:
	case PixelFormat::RGB_SRGB:
		return GL_RGB;
	default:
		return 0;
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

Image::~Image()
{
	if (m_id != 0)
		glDeleteTextures(1, &m_id);
}

Image::Image(Image && other)
{
	*this = std::move(other);
}

Image & Image::operator=(Image && other)
{
	if (this != &other)
	{
		if (m_id != 0)
			glDeleteTextures(1, &m_id);

		std::swap(m_id, other.m_id);
	}
	return *this;
}

void Image::Create()
{
	glGenTextures(1, &m_id);
}

Texture::Texture(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

std::expected<void, GraphicsError> Texture::Create(ImageData const & image_data, bool use_mip_map /*= true*/)
{
	if (!image_data.IsValid())
		return std::unexpected{ GraphicsError{ "Texture() image_data not valid" } };

	m_width = image_data.width;
	m_height = image_data.height;

	m_type = GL_TEXTURE_2D;
	m_image.Create();
	glBindTexture(m_type, m_image.GetId());

	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, use_mip_map ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum internal_format = to_gl_internal_format(image_data.format);
	GLenum format = to_gl_format(image_data.format);
	if (internal_format == 0 || format == 0)
		return std::unexpected{ GraphicsError{ "Texture() Unsupported pixel format: " + std::to_string(format) } };

	glTexImage2D(
		m_type,
		0 /*level*/,
		internal_format,
		m_width,
		m_height,
		0 /*border*/,
		format,
		GL_UNSIGNED_BYTE,
		image_data.data);

	if (use_mip_map)
		glGenerateMipmap(m_type);

	return {};
}

std::expected<void, GraphicsError> Texture::Create(CubeImageData const & image_data)
{
	if (!image_data.IsValid())
		return std::unexpected{ GraphicsError{ "Texture() image_data not valid" } };

	m_width = image_data.width;
	m_height = image_data.height;

	m_type = GL_TEXTURE_CUBE_MAP;
	m_image.Create();
	glBindTexture(m_type, m_image.GetId());

	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLenum internal_format = to_gl_internal_format(image_data.format);
	GLenum format = to_gl_format(image_data.format);
	if (internal_format == 0 || format == 0)
		return std::unexpected{ GraphicsError{ "Texture() Unsupported pixel format: " + std::to_string(format) } };

	for (unsigned int i = 0; i < image_data.data.size(); i++)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			internal_format,
			m_width,
			m_height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			image_data.data[i]);
	}

	return {};
}

bool Texture::IsValid() const
{
	return m_image.GetId() != 0 && m_type != 0;
}

void Texture::Bind(unsigned int binding) const
{
	if (!IsValid())
		return;

	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(m_type, m_image.GetId());
}
