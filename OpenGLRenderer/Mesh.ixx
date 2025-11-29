// Mesh.ixx

module;

#include <cstdint>
#include <expected>
#include <vector>

#include <glad/glad.h>

export module Mesh;

import Buffer;
import GraphicsApi;
import GraphicsError;
import VertexLayout;

class VertexArrayObject
{
public:
	VertexArrayObject() = default;
	~VertexArrayObject();

	VertexArrayObject(VertexArrayObject && other);
	VertexArrayObject & operator=(VertexArrayObject && other);

	VertexArrayObject(VertexArrayObject const &) = delete;
	VertexArrayObject & operator=(VertexArrayObject const &) = delete;

	void Create();

	unsigned int GetId() const { return m_id; }

private:
	unsigned int m_id = 0;
};

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
	Buffer m_element_buffer;
	VertexArrayObject m_vao;

	GLsizei m_index_count = 0;
};

Mesh::Mesh(GraphicsApi const & graphics_api)
	: m_graphics_api{ graphics_api }
{
}

template <Vertex::VertexWithLayout VertexT>
std::expected<void, GraphicsError> Mesh::Create(
	std::vector<VertexT> const & vertices,
	std::vector<IndexT> const & indices)
{
	if (vertices.empty() || indices.empty())
		return std::unexpected{ GraphicsError{ "Mesh::Create: invalid vertices or indicies." } };

	m_vertex_buffer.Create();
	m_element_buffer.Create();
	m_vao.Create();

	glBindVertexArray(m_vao.GetId());

	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer.GetId());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer.GetId());

	GLsizeiptr buffer_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexT));
	glBufferData(GL_ARRAY_BUFFER, buffer_size, vertices.data(), GL_STATIC_DRAW);

	buffer_size = static_cast<GLsizeiptr>(indices.size() * sizeof(IndexT));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, indices.data(), GL_STATIC_DRAW);

	Vertex::SetAttributes(VertexT::CreateLayout());

	glBindVertexArray(0);

	m_index_count = static_cast<GLsizei>(indices.size());

	return {};
}
