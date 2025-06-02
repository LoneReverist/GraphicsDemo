// ImageData.ixx

module;

#include <concepts>
#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

export module ImageData;

export class ImageData
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
		static_assert(std::same_as<stbi_uc, unsigned char>);

		//stbi_set_flip_vertically_on_load(true);

		m_data = stbi_load(filepath.string().c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
		if (!IsValid())
			std::cout << "ImageData::LoadImage() failed to load image: " << filepath << std::endl;
	}

	bool IsValid() const { return m_data != nullptr && m_width > 0 && m_height > 0; }

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	unsigned char * GetData() const { return m_data; }
	size_t GetSize() const { return static_cast<size_t>(m_width * m_height * 4); }

private:
	int m_width{ 0 };
	int m_height{ 0 };
	int m_channels{ 0 };
	unsigned char * m_data{ nullptr };
};
