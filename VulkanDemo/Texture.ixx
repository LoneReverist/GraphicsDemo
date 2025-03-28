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
	Texture(GraphicsApi const & graphics_api, std::filesystem::path const & filepath);
	Texture(GraphicsApi const & graphics_api, std::array<std::filesystem::path, 6> const & filepaths); // cubemap
	~Texture();

	bool IsValid() const;

	VkImageView GetImageView() const { return m_image_view; }
	VkSampler GetSampler() const { return m_sampler; }

private:
	GraphicsApi const & m_graphics_api;

	VkImage m_image{ VK_NULL_HANDLE };
	VkDeviceMemory m_image_memory{ VK_NULL_HANDLE };
	VkImageView m_image_view{ VK_NULL_HANDLE };
	VkSampler m_sampler{ VK_NULL_HANDLE };
};
