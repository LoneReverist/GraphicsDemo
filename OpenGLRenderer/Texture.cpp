// Texture.cpp

module;

#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>

#include <glad/glad.h>

module Texture;

GLenum to_gl_internal_format(PixelFormat format)
{
	if (format == PixelFormat::RGBA_UNORM)
		return GL_RGBA8;
	if (format == PixelFormat::RGB_UNORM)
		return GL_RGB8;
	if (format == PixelFormat::RGBA_SRGB)
		return GL_SRGB8_ALPHA8;
	if (format == PixelFormat::RGB_SRGB)
		return GL_SRGB8;
	throw std::runtime_error("to_gl_internal_format: Unexpected format");
	return 0;
}

GLenum to_gl_format(PixelFormat format)
{
	if (format == PixelFormat::RGBA_UNORM || format == PixelFormat::RGBA_SRGB)
		return GL_RGBA;
	if (format == PixelFormat::RGB_UNORM || format == PixelFormat::RGB_SRGB)
		return GL_RGB;
	throw std::runtime_error("to_gl_format: Unexpected format");
	return 0;
}

Tex::~Tex()
{
	if (m_id != 0)
		glDeleteTextures(1, &m_id);
}

Tex::Tex(Tex && other)
{
	*this = std::move(other);
}

Tex & Tex::operator=(Tex && other)
{
	if (this != &other)
	{
		if (m_id != 0)
			glDeleteTextures(1, &m_id);

		std::swap(m_id, other.m_id);
	}
	return *this;
}

void Tex::Create()
{
	glGenTextures(1, &m_id);
}

bool Texture::ImageData::IsValid() const
{
	return m_data != nullptr && m_width > 0 && m_height > 0;
}

std::uint64_t Texture::ImageData::GetSize() const
{
	return static_cast<std::uint64_t>(m_width * m_height * GetPixelSize(m_format));
}

bool Texture::CubeImageData::IsValid() const
{
	return std::ranges::all_of(m_data, [](std::uint8_t  const * data) { return data != nullptr; }) && m_width > 0 && m_height > 0;
}

std::uint64_t Texture::CubeImageData::GetSize() const
{
	return static_cast<std::uint64_t>(m_width * m_height * GetPixelSize(m_format));
}

Texture::Texture(GraphicsApi const & graphics_api, ImageData const & image_data, bool use_mip_map /*= true*/)
	: m_graphics_api(graphics_api)
{
	if (!image_data.IsValid())
		throw std::runtime_error("Texture() image_data not valid");

	m_type = GL_TEXTURE_2D;
	m_tex.Create();
	glBindTexture(m_type, m_tex.GetId());

	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, use_mip_map ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum internal_format = to_gl_internal_format(image_data.m_format);
	GLenum format = to_gl_format(image_data.m_format);
	glTexImage2D(
		m_type,
		0 /*level*/,
		internal_format,
		image_data.m_width,
		image_data.m_height,
		0 /*border*/,
		format,
		GL_UNSIGNED_BYTE,
		image_data.m_data);

	if (use_mip_map)
		glGenerateMipmap(m_type);
}

Texture::Texture(GraphicsApi const & graphics_api, CubeImageData const & image_data)
	: m_graphics_api(graphics_api)
{
	if (!image_data.IsValid())
		throw std::runtime_error("Texture() image_data not valid");

	m_type = GL_TEXTURE_CUBE_MAP;
	m_tex.Create();
	glBindTexture(m_type, m_tex.GetId());

	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLenum internal_format = to_gl_internal_format(image_data.m_format);
	GLenum format = to_gl_format(image_data.m_format);
	for (unsigned int i = 0; i < image_data.m_data.size(); i++)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			internal_format,
			image_data.m_width,
			image_data.m_height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			image_data.m_data[i]);
	}
}

bool Texture::IsValid() const
{
	return m_tex.GetId() != 0 && m_type != 0;
}

void Texture::Bind() const
{
	glBindTexture(m_type, m_tex.GetId());
}
