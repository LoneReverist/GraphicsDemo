// Mesh.cpp

module;

#include <cstdint>
#include <utility>

#include <glad/glad.h>

module Mesh;

VertexArrayObject::~VertexArrayObject()
{
	if (m_id != 0)
		glDeleteVertexArrays(1, &m_id);
}

VertexArrayObject::VertexArrayObject(VertexArrayObject && other)
{
	*this = std::move(other);
}

VertexArrayObject & VertexArrayObject::operator=(VertexArrayObject && other)
{
	if (this != &other)
	{
		if (m_id != 0)
			glDeleteVertexArrays(1, &m_id);

		std::swap(m_id, other.m_id);
	}
	return *this;
}

void VertexArrayObject::Create()
{
	glGenVertexArrays(1, &m_id);
}

bool Mesh::IsInitialized() const
{
	return m_vao.GetId() != 0
		&& m_vertex_buffer.GetId() != 0
		&& m_element_buffer.GetId() != 0
		&& m_index_count > 0;
}

void Mesh::Render() const
{
	if (!IsInitialized())
		return;

	glBindVertexArray(m_vao.GetId());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	static_assert(std::is_same_v<IndexT, std::uint16_t>,
		"Mesh::Render only supports 16-bit indices");
	glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_SHORT, nullptr);
}
