// Mesh.ixx

module;

#include <cstdint>
#include <cstring>
#include <expected>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

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
{
}

template <typename T>
Buffer create_buffer(
	GraphicsApi const & graphics_api,
	std::vector<T> objects,
	vk::BufferUsageFlagBits buffer_usage)
{
	VkDeviceSize buffer_size = sizeof(objects[0]) * objects.size();
	Buffer staging_buffer;
	staging_buffer.Create(
		graphics_api,
		buffer_size,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void * data = staging_buffer.GetMemory().mapMemory(0, buffer_size);
	std::memcpy(data, objects.data(), (size_t)buffer_size);
	staging_buffer.GetMemory().unmapMemory();
	
	Buffer out_buffer;
	out_buffer.Create(
		graphics_api,
		buffer_size,
		vk::BufferUsageFlagBits::eTransferDst | buffer_usage,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	graphics_api.CopyBuffer(staging_buffer.Get(), out_buffer.Get(), buffer_size);

	return out_buffer;
}

template<Vertex::VertexWithLayout VertexT>
std::expected<void, GraphicsError> Mesh::Create(
	std::vector<VertexT> const & vertices,
	std::vector<IndexT> const & indices)
{
	if (vertices.empty() || indices.empty())
		return std::unexpected{ GraphicsError{ "Mesh::Create: invalid vertices or indicies." } };

	try
	{
		m_vertex_buffer = create_buffer(
			m_graphics_api.get(),
			vertices,
			vk::BufferUsageFlagBits::eVertexBuffer);

		m_index_buffer = create_buffer(
			m_graphics_api.get(),
			indices,
			vk::BufferUsageFlagBits::eIndexBuffer);
	}
	catch (vk::SystemError const & err)
	{
		return std::unexpected{ GraphicsError{ "Mesh::Create: failed to create buffers. code: " + std::to_string(err.code().value()) } };
	}

	m_index_count = static_cast<std::uint32_t>(indices.size());

	return {};
}
