// Buffer.cpp

module;

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

module Buffer;

import GraphicsApi;
import GraphicsError;

void Buffer::Create(
	GraphicsApi const & graphics_api,
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties)
{
	m_memory.clear();
	m_buffer.clear();

	vk::raii::Device const & device = graphics_api.GetDevice();

	vk::BufferCreateInfo buffer_info{
		.size = size,
		.usage = usage,
		.sharingMode = vk::SharingMode::eExclusive,
	};

	m_buffer = vk::raii::Buffer{ device, buffer_info };

	vk::MemoryRequirements mem_requirements = m_buffer.getMemoryRequirements();
	std::uint32_t mem_type_index = graphics_api.FindMemoryType(mem_requirements.memoryTypeBits, properties);

	vk::MemoryAllocateInfo alloc_info{
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = mem_type_index
	};

	m_memory = vk::raii::DeviceMemory{ device, alloc_info };

	m_buffer.bindMemory(m_memory, 0);
}
