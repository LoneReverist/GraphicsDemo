// Mesh.ixx

module;

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

export module Mesh;

import Buffer;
import GraphicsApi;
import VertexLayout;

export class Mesh
{
public:
	using IndexT = std::uint16_t;

	template <Vertex::VertexWithLayout VertexT>
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
VkResult create_buffer(
	GraphicsApi const & graphics_api,
	std::vector<T> objects,
	VkBufferUsageFlags buffer_usage,
	Buffer & out_buffer)
{
	VkDevice device = graphics_api.GetDevice();

	VkDeviceSize buffer_size = sizeof(objects[0]) * objects.size();
	Buffer staging_buffer(graphics_api);
	VkResult result = staging_buffer.Create(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create staging buffer" << std::endl;
		return result;
	}

	void * data;
	vkMapMemory(device, staging_buffer.GetMemory(), 0, buffer_size, 0, &data);
	std::memcpy(data, objects.data(), (size_t)buffer_size);
	vkUnmapMemory(device, staging_buffer.GetMemory());

	result = out_buffer.Create(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer_usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create device local buffer" << std::endl;
		return result;
	}

	graphics_api.CopyBuffer(staging_buffer.Get(), out_buffer.Get(), buffer_size);

	return VK_SUCCESS;
}

template<Vertex::VertexWithLayout VertexT>
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

	VkResult result = create_buffer(
		m_graphics_api.get(),
		vertices,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		m_vertex_buffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create vertex buffer" << std::endl;
		return;
	}

	result = create_buffer(
		m_graphics_api.get(),
		indices,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		m_index_buffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create index buffer" << std::endl;
		return;
	}

	m_index_count = static_cast<std::uint32_t>(indices.size());
}
