// StbImage.ixx

module;

#include <concepts>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

export module StbImage;

export class StbImage
{
public:
	StbImage() = default;
	explicit StbImage(std::filesystem::path const & filepath, int req_comp);

	~StbImage();

	StbImage(StbImage const &) = delete;
	StbImage(StbImage const &&) = delete;
	StbImage & operator=(StbImage const &) = delete;
	StbImage & operator=(StbImage const &&) = delete;

	void LoadImage(std::filesystem::path const & filepath, int req_comp, bool flip_vertically = false);

	bool IsValid() const { return m_data != nullptr && m_width > 0 && m_height > 0; }

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	unsigned char * GetData() const { return m_data; }
	size_t GetSize() const { return static_cast<size_t>(m_width * m_height * 4); }

private:
	void free_image();

private:
	int m_width{ 0 };
	int m_height{ 0 };
	int m_channels{ 0 };
	std::uint8_t * m_data{ nullptr };
};

StbImage::StbImage(std::filesystem::path const & filepath, int req_comp)
{
	LoadImage(filepath, req_comp);
}

StbImage::~StbImage()
{
	free_image();
}

void StbImage::LoadImage(std::filesystem::path const & filepath, int req_comp, bool flip_vertically /*= false*/)
{
	static_assert(std::same_as<stbi_uc, unsigned char>);

	free_image();

	if (req_comp < 1)
		return;

	stbi_set_flip_vertically_on_load(flip_vertically);

	m_data = stbi_load(filepath.string().c_str(), &m_width, &m_height, &m_channels, req_comp);
}

void StbImage::free_image()
{
	if (m_data != nullptr)
	{
		stbi_image_free(m_data);
		m_data = nullptr;
	}
	m_width = 0;
	m_height = 0;
	m_channels = 0;
}
