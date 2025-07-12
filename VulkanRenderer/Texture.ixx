// Texture.ixx

module;

#include <array>
#include <filesystem>

#include <vulkan/vulkan.h>

export module Texture;

import GraphicsApi;

export class Texture
{
public:
	struct ImageData
	{
		std::uint8_t const * m_data = nullptr;
		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;

		bool IsValid() const
		{
			return m_data != nullptr && m_width > 0 && m_height > 0;
		}
		std::uint64_t GetSize() const
		{
			return static_cast<std::uint64_t>(m_width * m_height * 4); // assuming RGBA format
		}
	};

	struct CubeImageData
	{
		std::array<std::uint8_t const *, 6> m_data;
		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;

		bool IsValid() const
		{
			return std::ranges::all_of(m_data, [](std::uint8_t  const * data) { return data != nullptr; }) && m_width > 0 && m_height > 0;
		}
		std::uint64_t GetSize() const
		{
			return static_cast<std::uint64_t>(m_width * m_height * 4); // assuming RGBA format
		}
	};

public:
	Texture(GraphicsApi const & graphics_api, ImageData const & image_data);
	Texture(GraphicsApi const & graphics_api, CubeImageData const & image_data);
	~Texture();

	Texture(Texture && other);
	Texture & operator=(Texture && other);

	Texture(Texture &) = delete;
	Texture & operator=(Texture &) = delete;

	bool IsValid() const;

	VkImageView GetImageView() const { return m_image_view; }
	VkSampler GetSampler() const { return m_sampler; }

private:
	void destroy_texture();

private:
	GraphicsApi const & m_graphics_api;

	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_image_memory = VK_NULL_HANDLE;
	VkImageView m_image_view = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;
};
