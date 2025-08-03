// Buffer.cpp

module;

#include <iostream>
#include <cstdint>

#include <vulkan/vulkan.h>

module Buffer;

import GraphicsApi;

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
	, m_buffer(other.m_buffer)
	, m_memory(other.m_memory)
{
	other.m_buffer = VK_NULL_HANDLE;
	other.m_memory = VK_NULL_HANDLE;
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

VkResult Buffer::Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
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
	{
		std::cout << "Failed to create buffer" << std::endl;
		return result;
	}

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
	{
		std::cout << "Failed to allocate buffer memory" << std::endl;
		return result;
	}

	vkBindBufferMemory(device, m_buffer, m_memory, 0);
	return VK_SUCCESS;
}

void Buffer::Destroy()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroyBuffer(device, m_buffer, nullptr);
	m_buffer = VK_NULL_HANDLE;
	vkFreeMemory(device, m_memory, nullptr);
	m_memory = VK_NULL_HANDLE;
}
