// Texture.cpp

module;

#include <array>
#include <iostream>

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

module Texture;

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
		static_assert(std::same_as<stbi_uc, unsigned char>);

		//stbi_set_flip_vertically_on_load(true);

		m_data = stbi_load(filepath.string().c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
		if (!IsValid())
			std::cout << "ImageData::LoadImage() failed to load iamge: " << filepath << std::endl;
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

namespace
{
	void transition_image_layout(GraphicsApi const & graphics_api, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{
		graphics_api.DoOneTimeCommand([image, format, old_layout, new_layout](VkCommandBuffer command_buffer)
			{
				VkImageMemoryBarrier barrier{
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.srcAccessMask = 0,
					.dstAccessMask = 0,
					.oldLayout = old_layout,
					.newLayout = new_layout,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = image,
					.subresourceRange{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1,
					}
				};

				VkPipelineStageFlags src_stage;
				VkPipelineStageFlags dst_stage;

				if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

					src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				}
				else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
					dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else {
					throw std::invalid_argument("unsupported layout transition!");
				}

				vkCmdPipelineBarrier(
					command_buffer,
					src_stage, dst_stage,
					0 /*dependencyFlags*/,
					0 /*memoryBarrierCount*/, nullptr,
					0 /*bufferMemoryBarrierCount*/, nullptr,
					1 /*imageMemoryBarrierCount*/, &barrier
				);
			});
	}

	void copy_buffer_to_image(GraphicsApi const & graphics_api, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		graphics_api.DoOneTimeCommand([buffer, image, width, height](VkCommandBuffer command_buffer)
			{
				VkBufferImageCopy region{
					.bufferOffset = 0,
					.bufferRowLength = 0,
					.bufferImageHeight = 0,
					.imageSubresource{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = 0,
						.baseArrayLayer = 0,
						.layerCount = 1,
					},
					.imageOffset{ 0, 0, 0 },
					.imageExtent{ width, height, 1 }
				};

				vkCmdCopyBufferToImage(
					command_buffer,
					buffer,
					image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			});
	}
}

Texture::Texture(GraphicsApi const & graphics_api, std::filesystem::path const & filepath)
	: m_graphics_api(graphics_api)
{
	VkDevice device = m_graphics_api.GetDevice();

	VkBuffer staging_buffer = VK_NULL_HANDLE;
	VkDeviceMemory staging_buffer_memory = VK_NULL_HANDLE;
	uint32_t width = 0;
	uint32_t height = 0;
	{
		ImageData image{ filepath };
		if (!image.IsValid())
			return;

		width = static_cast<uint32_t>(image.GetWidth());
		height = static_cast<uint32_t>(image.GetHeight());

		VkResult result = graphics_api.CreateBuffer(static_cast<VkDeviceSize>(image.GetSize()),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_buffer_memory);
		if (result != VK_SUCCESS)
		{
			std::cout << "Texture::LoadTexture() Failed to create staging buffer: " << filepath << std::endl;
			return;
		}

		void * data = nullptr;
		vkMapMemory(device, staging_buffer_memory, 0, static_cast<VkDeviceSize>(image.GetSize()), 0, &data);
		memcpy(data, image.GetData(), image.GetSize());
		vkUnmapMemory(device, staging_buffer_memory);
	} // image goes out of scope and frees memory

	VkResult result = graphics_api.Create2dImage(
		width,
		height,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_texture_image,
		m_texture_image_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Texture::LoadTexture() Failed to create image: " << filepath << std::endl;
		return;
	}

	transition_image_layout(
		m_graphics_api,
		m_texture_image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_buffer_to_image(
		m_graphics_api,
		staging_buffer,
		m_texture_image,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height));
	transition_image_layout(
		m_graphics_api,
		m_texture_image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);
}

//bool Texture::LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths)
//{
//	std::array<ImageData, 6> images;
//	for (unsigned int i = 0; i < filepaths.size(); i++)
//		images[i].LoadImage(filepaths[i]);
//
//	if (std::ranges::any_of(images, [](ImageData const & image) { return !image.IsValid(); }))
//		return false;
//
//	m_type = GL_TEXTURE_CUBE_MAP;
//	glGenTextures(1, &m_tex_id);
//	glBindTexture(m_type, m_tex_id);
//
//	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(m_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//	for (unsigned int i = 0; i < images.size(); i++)
//	{
//		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//			0, GL_RGB, images[i].GetWidth(), images[i].GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, images[i].GetData());
//	}
//
//	return true;
//}

Texture::~Texture()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroyImage(device, m_texture_image, nullptr);
	vkFreeMemory(device, m_texture_image_memory, nullptr);
}

void Texture::Bind() const
{
	//glBindTexture(m_type, m_tex_id);
}
