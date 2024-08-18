// Texture.cpp

#include "stdafx.h"
#include "Texture.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool Texture::LoadTexture(std::filesystem::path filepath)
{
	int width = 0, height = 0, nr_channels = 0;
	unsigned char * data = stbi_load(filepath.string().c_str(), &width, &height, &nr_channels, 0);
	if (!data || width == 0 || height == 0)
	{
		std::cout << "Texture::LoadTexture() failed to load texture" << std::endl;
		return false;
	}

	glGenTextures(1, &m_tex_id);
	glBindTexture(GL_TEXTURE_2D, m_tex_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0 /*level*/, GL_RGB, width, height, 0 /*border*/, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
	return true;
}

void Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_tex_id);
}
