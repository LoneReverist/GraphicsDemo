// Texture.cpp

module;

#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>

#include <glad/glad.h>

module Texture;

Texture::Texture(ImageData const & image_data)
{
	if (!image_data.IsValid())
		throw std::runtime_error("Texture() image_data not valid");

	m_type = GL_TEXTURE_2D;
	glGenTextures(1, &m_tex_id);
	glBindTexture(m_type, m_tex_id);

	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(m_type, 0 /*level*/, GL_RGBA, image_data.m_width, image_data.m_height, 0 /*border*/, GL_RGBA, GL_UNSIGNED_BYTE, image_data.m_data);
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

	for (unsigned int i = 0; i < image_data.m_data.size(); i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGBA, image_data.m_width, image_data.m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.m_data[i]);
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
