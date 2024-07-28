// RenderObject.cpp

#include "stdafx.h"
#include "RenderObject.h"

#include <glad/glad.h>

#include "ShaderProgram.h"

RenderObject::~RenderObject()
{
	glDeleteVertexArrays(1, &m_vao_id);
	glDeleteBuffers(1, &m_vbo_id);
	glDeleteBuffers(1, &m_ebo_id);
}

void RenderObject::Init()
{
	load_mesh();
	if (m_verts.empty())
		return;

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

	void * color_offset = reinterpret_cast<void *>(sizeof(float) * 3);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, color_offset);
	glEnableVertexAttribArray(1);
}

void RenderObject::Render() const
{
	if (m_vao_id == 0)
	{
		std::cout << "Mesh::Render() vertex array object not initialized" << std::endl;
		return;
	}
	if (!m_shader_program)
	{
		std::cout << "Mesh::Render() using invalid shader program" << std::endl;
		return;
	}

	glBindVertexArray(m_vao_id);

	if (m_draw_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_shader_program->Activate();

	GLsizei num_elements = static_cast<GLsizei>(m_indices.size());
	glDrawElements(GL_TRIANGLES, num_elements, GL_UNSIGNED_INT, nullptr);
}

void RenderObject::load_mesh()
{
	m_verts = { // x, y, z, r, g, b
		{  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f },
		{  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f },
		{ -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f },
		{ -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }
	};
	m_indices = {
		0, 1, 3,
		1, 2, 3
	};
}
