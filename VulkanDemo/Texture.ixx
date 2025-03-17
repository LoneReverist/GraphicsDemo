// Texture.ixx

module;

#include <filesystem>

#include <vulkan/vulkan.h>

export module Texture;

import GraphicsApi;

export class Texture
{
public:
	bool LoadTexture(GraphicsApi const & graphics_api, std::filesystem::path const & filepath);
	//bool LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths);

	void Bind() const;

private:
	VkImage m_texture_image;
	VkDeviceMemory m_texture_image_memory;

	int m_type{ -1 };
	unsigned int m_tex_id{ 0 };
};
