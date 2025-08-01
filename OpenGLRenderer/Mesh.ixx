// Mesh.ixx

module;

#include <cstdint>
#include <iostream>
#include <vector>

#include <glad/glad.h>

export module Mesh;

import GraphicsApi;
import Vertex;

export class Mesh
{
public:
	using IndexT = std::uint16_t;

	template <IsVertex VertexT>
	Mesh(
		GraphicsApi const & graphics_api,
		std::vector<VertexT> const & vertices,
		std::vector<IndexT> const & indices);
	~Mesh();

	Mesh(Mesh && other);
	Mesh & operator=(Mesh && other);

	Mesh(Mesh &) = delete;
	Mesh & operator=(Mesh &) = delete;

	bool IsInitialized() const;

	void Render() const;

private:
	void destroy_buffers();

private:
	GraphicsApi const & m_graphics_api;

	unsigned int m_vbo_id = 0; // vertex buffer object
	unsigned int m_ebo_id = 0; // element buffer object
	unsigned int m_vao_id = 0; // vertex array object

	GLsizei m_index_count = 0;
};

template <IsVertex VertexT>
Mesh::Mesh(
	GraphicsApi const & graphics_api,
	std::vector<VertexT> const & vertices,
	std::vector<IndexT> const & indices)
	: m_graphics_api{ graphics_api }
{
	if (vertices.empty())
	{
		//std::cout << "Mesh verts empty" << std::endl;
		return;
	}
	if (indices.empty())
	{
		//std::cout << "Mesh indices empty" << std::endl;
		return;
	}

	glGenBuffers(1, &m_vbo_id);
	glGenBuffers(1, &m_ebo_id);
	glGenVertexArrays(1, &m_vao_id);

	glBindVertexArray(m_vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_id);

	GLsizeiptr buffer_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexT));
	glBufferData(GL_ARRAY_BUFFER, buffer_size, vertices.data(), GL_STATIC_DRAW);

	buffer_size = static_cast<GLsizeiptr>(indices.size() * sizeof(IndexT));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, indices.data(), GL_STATIC_DRAW);

	Vertex::SetAttributes<VertexT>();

	glBindVertexArray(0);

	m_index_count = static_cast<GLsizei>(indices.size());
}

Mesh::~Mesh()
{
	destroy_buffers();
}

void Mesh::destroy_buffers()
{
	glDeleteVertexArrays(1, &m_vao_id);
	glDeleteBuffers(1, &m_vbo_id);
	glDeleteBuffers(1, &m_ebo_id);
	m_vao_id = 0;
	m_vbo_id = 0;
	m_ebo_id = 0;
}

Mesh::Mesh(Mesh && other)
	: m_graphics_api(other.m_graphics_api)
{
	*this = std::move(other);
}

Mesh & Mesh::operator=(Mesh && other)
{
	if (this == &other)
		return *this;

	destroy_buffers();

	m_vbo_id = other.m_vbo_id;
	m_ebo_id = other.m_ebo_id;
	m_vao_id = other.m_vao_id;
	m_index_count = other.m_index_count;

	other.m_vao_id = 0;
	other.m_vbo_id = 0;
	other.m_ebo_id = 0;
	other.m_index_count = 0;

	return *this;
}

bool Mesh::IsInitialized() const
{
	return m_vao_id != 0
		&& m_vbo_id != 0
		&& m_ebo_id != 0
		&& m_index_count > 0;
}

void Mesh::Render() const
{
	if (!IsInitialized())
		return;

	glBindVertexArray(m_vao_id);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	static_assert(std::is_same_v<IndexT, std::uint16_t>,
		"Mesh::Render only supports 16-bit indices");
	glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_SHORT, nullptr);
}
