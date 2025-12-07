// Mesh.ixx

module;

#include <cstdint>
#include <cstring>
#include <expected>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

export module Mesh;

import Buffer;
import GraphicsApi;
import GraphicsError;
import VertexLayout;

export class Mesh
{
public:
	using IndexT = std::uint16_t;

	explicit Mesh(GraphicsApi const & graphics_api);
	~Mesh() = default;

	Mesh(Mesh && other) = default;
	Mesh & operator=(Mesh && other) = default;

	Mesh(Mesh const &) = delete;
	Mesh & operator=(Mesh const &) = delete;

	template <Vertex::VertexWithLayout VertexT>
	std::expected<void, GraphicsError> Create(
		std::vector<VertexT> const & vertices,
		std::vector<IndexT> const & indices);

	bool IsInitialized() const;

	void Render() const;

private:
	std::reference_wrapper<GraphicsApi const> m_graphics_api;

	Buffer m_vertex_buffer;
	Buffer m_index_buffer;

	std::uint32_t m_index_count = 0;
};

Mesh::Mesh(GraphicsApi const & graphics_api)
	: m_graphics_api{ graphics_api }
	, m_vertex_buffer{ graphics_api }
	, m_index_buffer{ graphics_api }
{
}

template <typename T>
std::expected<void, GraphicsError> create_buffer(
	GraphicsApi const & graphics_api,
	std::vector<T> objects,
	VkBufferUsageFlags buffer_usage,
	Buffer & out_buffer)
{
	VkDevice device = graphics_api.GetDevice();

	VkDeviceSize buffer_size = sizeof(objects[0]) * objects.size();
	Buffer staging_buffer(graphics_api);
	std::expected<void, GraphicsError> result = staging_buffer.Create(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	if (!result.has_value())
		return std::unexpected{ result.error().AddToMessage(" Mesh::create_buffer: Failed to create staging buffer.") };

	void * data;
	vkMapMemory(device, staging_buffer.GetMemory(), 0, buffer_size, 0, &data);
	std::memcpy(data, objects.data(), (size_t)buffer_size);
	vkUnmapMemory(device, staging_buffer.GetMemory());

	result = out_buffer.Create(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer_usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (!result.has_value())
		return std::unexpected{ result.error().AddToMessage(" Mesh::create_buffer: Failed to create device local buffer.") };

	graphics_api.CopyBuffer(staging_buffer.Get(), out_buffer.Get(), buffer_size);

	return {};
}

template<Vertex::VertexWithLayout VertexT>
std::expected<void, GraphicsError> Mesh::Create(
	std::vector<VertexT> const & vertices,
	std::vector<IndexT> const & indices)
{
	if (vertices.empty() || indices.empty())
		return std::unexpected{ GraphicsError{ "Mesh::Create: invalid vertices or indicies." } };

	std::expected<void, GraphicsError> v_buf_result = create_buffer(
		m_graphics_api.get(),
		vertices,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		m_vertex_buffer);
	if (!v_buf_result.has_value())
		return std::unexpected{ v_buf_result.error().AddToMessage(" Mesh::Create: Failed to create vertex buffer.") };

	std::expected<void, GraphicsError> i_buf_result = create_buffer(
		m_graphics_api.get(),
		indices,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		m_index_buffer);
	if (!i_buf_result.has_value())
		return std::unexpected{ i_buf_result.error().AddToMessage(" Mesh::Create: Failed to create index buffer.") };

	m_index_count = static_cast<std::uint32_t>(indices.size());

	return {};
}
