// Texture.cpp

module;

#include <array>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

module Texture;

//import <array>;
import <iostream>;

class ImageData
{
public:
	ImageData() = default;
	ImageData(std::filesystem::path const & filepath)
	{
		LoadImage(filepath);
	}

	~ImageData()
	{
		if (m_data != nullptr)
			stbi_image_free(m_data);
	}

	void LoadImage(std::filesystem::path const & filepath)
	{
		stbi_set_flip_vertically_on_load(true);

		m_data = stbi_load(filepath.string().c_str(), &m_width, &m_height, &m_nr_channels, 0);
		if (!IsValid())
			std::cout << "Texture::LoadTexture() failed to load texture: " << filepath << std::endl;
	}

	bool IsValid() const { return m_data != nullptr && m_width > 0 && m_height > 0; }

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	unsigned char * GetData() const { return m_data; }

private:
	int m_width{ 0 };
	int m_height{ 0 };
	int m_nr_channels{ 0 };
	unsigned char * m_data{ nullptr };
};

bool Texture::LoadTexture(std::filesystem::path const & filepath)
{
	ImageData image(filepath);
	if (!image.IsValid())
		return false;

	m_type = GL_TEXTURE_2D;
	glGenTextures(1, &m_tex_id);
	glBindTexture(m_type, m_tex_id);

	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(m_type, 0 /*level*/, GL_RGB, image.GetWidth(), image.GetHeight(), 0 /*border*/, GL_RGB, GL_UNSIGNED_BYTE, image.GetData());
	glGenerateMipmap(m_type);

	return true;
}

bool Texture::LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths)
{
	std::array<ImageData, 6> images;
	for (unsigned int i = 0; i < filepaths.size(); i++)
		images[i].LoadImage(filepaths[i]);

	if (std::ranges::any_of(images, [](ImageData const & image) { return !image.IsValid(); }))
		return false;

	m_type = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &m_tex_id);
	glBindTexture(m_type, m_tex_id);

	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < images.size(); i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, images[i].GetWidth(), images[i].GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, images[i].GetData());
	}

	return true;
}

void Texture::Bind() const
{
	glBindTexture(m_type, m_tex_id);
}
