// Buffer.cpp

module;

#include <cstdint>
#include <expected>
#include <string>

#include <vulkan/vulkan.h>

module Buffer;

import GraphicsApi;
import GraphicsError;

Buffer::Buffer(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

Buffer::~Buffer()
{
	Destroy();
}

Buffer::Buffer(Buffer && other) noexcept
	: m_graphics_api(other.m_graphics_api)
{
	std::swap(m_buffer, other.m_buffer);
	std::swap(m_memory, other.m_memory);
}

Buffer & Buffer::operator=(Buffer && other) noexcept
{
	if (this != &other)
	{
		Destroy();
		std::swap(m_buffer, other.m_buffer);
		std::swap(m_memory, other.m_memory);
	}
	return *this;
}

std::expected<void, GraphicsError> Buffer::Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	Destroy();

	VkDevice device = m_graphics_api.GetDevice();

	VkBufferCreateInfo buffer_info{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	VkResult result = vkCreateBuffer(device, &buffer_info, nullptr, &m_buffer);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "vkCreateBuffer failed. code: " + std::to_string(result) } };

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(device, m_buffer, &mem_requirements);

	std::uint32_t mem_type_index = m_graphics_api.FindMemoryType(mem_requirements.memoryTypeBits, properties);

	VkMemoryAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = mem_type_index
	};

	result = vkAllocateMemory(device, &alloc_info, nullptr, &m_memory);
	if (result != VK_SUCCESS)
		return std::unexpected{ GraphicsError{ "vkAllocateMemory failed. code: " + std::to_string(result) } };

	vkBindBufferMemory(device, m_buffer, m_memory, 0);

	return {};
}

void Buffer::Destroy()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroyBuffer(device, m_buffer, nullptr);
	m_buffer = VK_NULL_HANDLE;
	vkFreeMemory(device, m_memory, nullptr);
	m_memory = VK_NULL_HANDLE;
}
