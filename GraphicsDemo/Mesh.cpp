// Mesh.cpp

#include "stdafx.h"
#include "Mesh.h"

#include <glad/glad.h>

#include "ShaderProgram.h"

void Mesh::InitBuffers()
{
	if (m_verts.empty())
	{
		std::cout << "Mesh::Init() verts empty" << std::endl;
		return;
	}
	if (m_indices.empty())
	{
		std::cout << "Mesh::Init() indices empty" << std::endl;
		return;
	}

	glGenBuffers(1, &m_vbo_id);
	glGenBuffers(1, &m_ebo_id);
	glGenVertexArrays(1, &m_vao_id);

	glBindVertexArray(m_vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_id);

	GLsizeiptr buffer_size = static_cast<GLsizeiptr>(m_verts.size() * sizeof(Vertex));
	glBufferData(GL_ARRAY_BUFFER, buffer_size, m_verts.data(), GL_STATIC_DRAW);

	buffer_size = static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned int));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, m_indices.data(), GL_STATIC_DRAW);

	GLsizei stride = sizeof(Vertex);
	void * pos_offset = reinterpret_cast<void *>(0);
	glVertexAttribPointer(
		0,				// layout (location = 0) in vertex shader
		3,				// 3 floats per vertex position
		GL_FLOAT,		// data type
		GL_FALSE,		// normalize (not relevant)
		stride,			// size of vertex
		pos_offset);	// offset
	glEnableVertexAttribArray(0);

	void * normal_offset = reinterpret_cast<void *>(sizeof(glm::vec3));
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		stride,
		normal_offset);
	glEnableVertexAttribArray(1);
}

void Mesh::DeleteBuffers()
{
	glDeleteVertexArrays(1, &m_vao_id);
	glDeleteBuffers(1, &m_vbo_id);
	glDeleteBuffers(1, &m_ebo_id);
	m_vao_id = 0;
	m_vbo_id = 0;
	m_ebo_id = 0;
}

void Mesh::Render(bool wireframe) const
{
	if (m_vao_id == 0)
		return;

	glBindVertexArray(m_vao_id);

	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT, GL_FILL);

	GLsizei num_elements = static_cast<GLsizei>(m_indices.size());
	glDrawElements(GL_TRIANGLES, num_elements, GL_UNSIGNED_INT, nullptr);
}
