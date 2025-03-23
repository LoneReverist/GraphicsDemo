// Texture.ixx

module;

#include <filesystem>

#include <vulkan/vulkan.h>

export module Texture;

import GraphicsApi;

export class Texture
{
public:
	Texture(GraphicsApi const & graphics_api, std::filesystem::path const & filepath);
	~Texture();

	bool IsValid() const;

	//bool LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths);

	void Bind() const;

private:
	GraphicsApi const & m_graphics_api;

	VkImage m_image{ VK_NULL_HANDLE };
	VkDeviceMemory m_image_memory{ VK_NULL_HANDLE };
	VkImageView m_image_view{ VK_NULL_HANDLE };
	VkSampler m_sampler{ VK_NULL_HANDLE };

	int m_type{ -1 };
	unsigned int m_tex_id{ 0 };
};
