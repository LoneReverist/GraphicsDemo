// Texture.cpp

module;

#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>

#include <glad/glad.h>

module Texture;

GLenum to_gl_format(PixelFormat format)
{
	if (format == PixelFormat::RGBA)
		return GL_RGBA;
	if (format == PixelFormat::RGB)
		return GL_RGB;
	throw std::runtime_error("to_gl_format: Unexpected format");
	return 0;
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

Texture::Texture(ImageData const & image_data, bool use_mip_map /*= true*/)
{
	if (!image_data.IsValid())
		throw std::runtime_error("Texture() image_data not valid");

	m_type = GL_TEXTURE_2D;
	glGenTextures(1, &m_tex_id);
	glBindTexture(m_type, m_tex_id);

	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, use_mip_map ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum format = to_gl_format(image_data.m_format);
	glTexImage2D(
		m_type,
		0 /*level*/,
		format,
		image_data.m_width,
		image_data.m_height,
		0 /*border*/,
		format,
		GL_UNSIGNED_BYTE,
		image_data.m_data);

	if (use_mip_map)
		glGenerateMipmap(m_type);
}

Texture::Texture(CubeImageData const & image_data)
{
	if (!image_data.IsValid())
		throw std::runtime_error("Texture() image_data not valid");

	m_type = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &m_tex_id);
	glBindTexture(m_type, m_tex_id);

	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLenum format = to_gl_format(image_data.m_format);
	for (unsigned int i = 0; i < image_data.m_data.size(); i++)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			format,
			image_data.m_width,
			image_data.m_height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			image_data.m_data[i]);
	}
}

Texture::~Texture()
{
	destroy_texture();
}

void Texture::destroy_texture()
{
	glDeleteTextures(1, &m_tex_id);
	m_tex_id = 0;
	m_type = 0;
}

Texture::Texture(Texture && other)
{
	*this = std::move(other);
}

Texture & Texture::operator=(Texture && other)
{
	if (this == &other)
		return *this;

	destroy_texture();

	m_tex_id = other.m_tex_id;
	m_type = other.m_type;

	other.m_tex_id = 0;
	other.m_type = 0;

	return *this;
}

bool Texture::IsValid() const
{
	return m_tex_id != 0 && m_type != 0;
}

void Texture::Bind() const
{
	glBindTexture(m_type, m_tex_id);
}
