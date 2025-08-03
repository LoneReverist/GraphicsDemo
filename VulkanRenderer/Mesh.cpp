// Mesh.cpp

module;

#include <utility>

#include <vulkan/vulkan.h>

module Mesh;

Buffer::~Buffer()
{
	destroy();
}

Buffer::Buffer(Buffer && other)
	: m_graphics_api(other.m_graphics_api)
{
	*this = std::move(other);
}

Buffer & Buffer::operator=(Buffer && other)
{
	if (this != &other)
	{
		destroy();

		std::swap(m_buffer, other.m_buffer);
		std::swap(m_buffer_memory, other.m_buffer_memory);
	}
	return *this;
}

void Buffer::destroy()
{
	VkDevice device = m_graphics_api.get().GetDevice();

	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, m_buffer_memory, nullptr);
		m_buffer_memory = VK_NULL_HANDLE;
	}
}

bool Mesh::IsInitialized() const
{
	return m_vertex_buffer.Get() != VK_NULL_HANDLE
		&& m_vertex_buffer.GetMemory() != VK_NULL_HANDLE
		&& m_index_buffer.Get() != VK_NULL_HANDLE
		&& m_index_buffer.GetMemory() != VK_NULL_HANDLE
		&& m_index_count > 0;
}

void Mesh::Render() const
{
	if (!IsInitialized())
		return;

	VkCommandBuffer command_buffer = m_graphics_api.get().GetCurCommandBuffer();

	VkBuffer vertex_buffers[] = { m_vertex_buffer.Get() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(
		command_buffer,
		0 /*firstBinding*/,
		1 /*bindingCount*/,
		vertex_buffers,
		offsets);

	static_assert(std::same_as<IndexT, std::uint16_t>);
	vkCmdBindIndexBuffer(
		command_buffer,
		m_index_buffer.Get(),
		0 /*offset*/,
		VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(
		command_buffer,
		m_index_count,
		1 /*instanceCount*/,
		0 /*firstIndex*/,
		0 /*vertexOffset*/,
		0 /*firstInstance*/);
}
