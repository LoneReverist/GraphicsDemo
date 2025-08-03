// Mesh.ixx

module;

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

export module Mesh;

import GraphicsApi;
import Vertex;

class Buffer
{
public:
	Buffer(GraphicsApi const & graphics_api) : m_graphics_api(graphics_api) {}
	~Buffer();

	Buffer(Buffer && other);
	Buffer & operator=(Buffer && other);

	Buffer(Buffer const &) = delete;
	Buffer & operator=(Buffer const &) = delete;

	template <typename T>
	VkResult Create(
		GraphicsApi const & graphics_api,
		std::vector<T> objects,
		VkBufferUsageFlags buffer_usage);

	VkBuffer Get() const { return m_buffer; }
	VkDeviceMemory GetMemory() const { return m_buffer_memory; }

private:
	void destroy();

private:
	std::reference_wrapper<GraphicsApi const> m_graphics_api;

	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_buffer_memory = VK_NULL_HANDLE;
};

export class Mesh
{
public:
	using IndexT = std::uint16_t;

	template <IsVertex VertexT>
	Mesh(
		GraphicsApi const & graphics_api,
		std::vector<VertexT> const & vertices,
		std::vector<IndexT> const & indices);
	~Mesh() = default;

	Mesh(Mesh && other) = default;
	Mesh & operator=(Mesh && other) = default;

	Mesh(Mesh const &) = delete;
	Mesh & operator=(Mesh const &) = delete;

	bool IsInitialized() const;

	void Render() const;

private:
	std::reference_wrapper<GraphicsApi const> m_graphics_api;

	Buffer m_vertex_buffer;
	Buffer m_index_buffer;

	std::uint32_t m_index_count = 0;
};

template <typename T>
VkResult Buffer::Create(
	GraphicsApi const & graphics_api,
	std::vector<T> objects,
	VkBufferUsageFlags buffer_usage)
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
	std::memcpy(data, objects.data(), (size_t)buffer_size);
	vkUnmapMemory(device, staging_buffer_memory);

	result = graphics_api.CreateBuffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer_usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_buffer,
		m_buffer_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create device local buffer" << std::endl;
		return result;
	}

	graphics_api.CopyBuffer(staging_buffer, m_buffer, buffer_size);

	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);

	return VK_SUCCESS;
}

template<IsVertex VertexT>
Mesh::Mesh(
	GraphicsApi const & graphics_api,
	std::vector<VertexT> const & vertices,
	std::vector<IndexT> const & indices)
	: m_graphics_api{ graphics_api }
	, m_vertex_buffer{ graphics_api }
	, m_index_buffer{ graphics_api }
{
	if (vertices.empty() || indices.empty())
		return;

	VkResult result = m_vertex_buffer.Create(
		m_graphics_api,
		vertices,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create vertex buffer" << std::endl;
		return;
	}

	result = m_index_buffer.Create(
		m_graphics_api,
		indices,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create index buffer" << std::endl;
		return;
	}

	m_index_count = static_cast<std::uint32_t>(indices.size());
}
