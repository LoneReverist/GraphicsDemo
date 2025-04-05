// Mesh.ixx

module;

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

export module Mesh;

import GraphicsApi;
import Vertex;

export class Mesh
{
public:
	using index_t = std::uint16_t;

	template <IsVertex Vert>
	Mesh(
		GraphicsApi const & graphics_api,
		std::vector<Vert> const & vertices,
		std::vector<index_t> const & indices);
	Mesh(Mesh && other);
	~Mesh();

	Mesh(Mesh &) = delete;
	Mesh & operator=(Mesh &) = delete;
	Mesh & operator=(Mesh &&) = delete;

	bool IsInitialized() const;

	void Render(bool wireframe) const;

private:
	GraphicsApi const & m_graphics_api;

	VkBuffer m_vertex_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertex_buffer_memory = VK_NULL_HANDLE;
	VkBuffer m_index_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_index_buffer_memory = VK_NULL_HANDLE;

	std::uint32_t m_index_count = 0;
};

namespace
{
	template <typename T>
	VkResult init_buffer(
		GraphicsApi const & graphics_api,
		std::vector<T> objects,
		VkBufferUsageFlags buffer_usage,
		VkBuffer & out_buffer,
		VkDeviceMemory & out_buffer_memory
		)
	{
		VkDevice device = graphics_api.GetDevice();

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		VkDeviceSize buffer_size = sizeof(objects[0]) * objects.size();
		VkResult result = graphics_api.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_buffer_memory);
		if (result != VK_SUCCESS)
		{
			std::cout << "Failed to create staging buffer" << std::endl;
			return result;
		}

		void * data;
		vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, objects.data(), (size_t)buffer_size);
		vkUnmapMemory(device, staging_buffer_memory);

		result = graphics_api.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer_usage,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			out_buffer,
			out_buffer_memory);
		if (result != VK_SUCCESS)
		{
			std::cout << "Failed to create device local buffer" << std::endl;
			return result;
		}

		graphics_api.CopyBuffer(staging_buffer, out_buffer, buffer_size);

		vkDestroyBuffer(device, staging_buffer, nullptr);
		vkFreeMemory(device, staging_buffer_memory, nullptr);

		return VK_SUCCESS;
	}
}

template<IsVertex Vert>
Mesh::Mesh(
	GraphicsApi const & graphics_api,
	std::vector<Vert> const & vertices,
	std::vector<index_t> const & indices)
	: m_graphics_api{ graphics_api }
{
	if (vertices.empty())
	{
		std::cout << "Mesh verts empty" << std::endl;
		return;
	}
	if (indices.empty())
	{
		std::cout << "Mesh indices empty" << std::endl;
		return;
	}

	VkResult result = init_buffer(
		m_graphics_api,
		vertices,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		m_vertex_buffer,
		m_vertex_buffer_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create vertex buffer" << std::endl;
		return;
	}

	result = init_buffer(
		m_graphics_api,
		indices,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		m_index_buffer,
		m_index_buffer_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create index buffer" << std::endl;
		return;
	}

	m_index_count = static_cast<std::uint32_t>(indices.size());
}

Mesh::Mesh(Mesh && other)
	: m_graphics_api(other.m_graphics_api)
{
	std::swap(m_vertex_buffer, other.m_vertex_buffer);
	std::swap(m_vertex_buffer_memory, other.m_vertex_buffer_memory);
	std::swap(m_index_buffer, other.m_index_buffer);
	std::swap(m_index_buffer_memory, other.m_index_buffer_memory);
	std::swap(m_index_count, other.m_index_count);
}

Mesh::~Mesh()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroyBuffer(device, m_index_buffer, nullptr);
	m_index_buffer = VK_NULL_HANDLE;
	vkFreeMemory(device, m_index_buffer_memory, nullptr);
	m_index_buffer_memory = VK_NULL_HANDLE;

	vkDestroyBuffer(device, m_vertex_buffer, nullptr);
	m_vertex_buffer = VK_NULL_HANDLE;
	vkFreeMemory(device, m_vertex_buffer_memory, nullptr);
	m_vertex_buffer_memory = VK_NULL_HANDLE;
}

bool Mesh::IsInitialized() const
{
	return m_vertex_buffer != VK_NULL_HANDLE
		&& m_vertex_buffer_memory != VK_NULL_HANDLE
		&& m_index_buffer != VK_NULL_HANDLE
		&& m_index_buffer_memory != VK_NULL_HANDLE
		&& m_index_count > 0;
}

void Mesh::Render(bool /*wireframe*/) const
{
	if (!IsInitialized())
		return;

	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();

	VkBuffer vertex_buffers[] = { m_vertex_buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(
		command_buffer,
		0 /*firstBinding*/,
		1 /*bindingCount*/,
		vertex_buffers,
		offsets);

	static_assert(std::same_as<index_t, std::uint16_t>);
	vkCmdBindIndexBuffer(
		command_buffer,
		m_index_buffer,
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
