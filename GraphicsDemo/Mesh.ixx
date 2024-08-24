// Mesh.ixx
module;

#include "stdafx.h"

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

export module Mesh;

export struct PositionVertex {
	glm::vec3 m_pos;
};

export struct NormalVertex {
	glm::vec3 m_pos;
	glm::vec3 m_normal;
};

export struct TextureVertex {
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec2 m_tex_coord;
};

template <typename T>
concept VertexConcept = std::same_as<T, PositionVertex> || std::same_as<T, NormalVertex> || std::same_as<T, TextureVertex>;

template <typename T>
concept VertexSupportsNormal = VertexConcept<T> && requires(T v) { v.m_normal; };

template <typename T>
concept VertexSupportsTexCoord = VertexConcept<T> && requires(T v) { v.m_tex_coord; };

template <VertexConcept Vertex>
class MeshImpl
{
public:
	template<VertexConcept Vertex>
	MeshImpl(std::vector<Vertex> && verts, std::vector<unsigned int> && indices)
		: m_verts{ std::move(verts) }
		, m_indices{ std::move(indices) }
	{}

	void InitBuffers();
	void DeleteBuffers();

	void Render(bool wireframe) const;

private:
	std::vector<Vertex> m_verts;
	std::vector<unsigned int> m_indices;

	unsigned int m_vbo_id{ 0 }; // vertex buffer object
	unsigned int m_ebo_id{ 0 }; // element buffer object
	unsigned int m_vao_id{ 0 }; // vertex array object
};

export class Mesh
{
public:
	template<VertexConcept Vertex>
	Mesh(std::vector<Vertex> && verts, std::vector<unsigned int> && indices)
		: m_mesh_var(MeshImpl<Vertex>{ std::move(verts), std::move(indices) })
	{}

	void InitBuffers()
	{
		std::visit([](auto & mesh) { mesh.InitBuffers(); }, m_mesh_var);
	}
	void DeleteBuffers()
	{
		std::visit([](auto & mesh) { mesh.DeleteBuffers(); }, m_mesh_var);
	}

	void Render(bool wireframe) const
	{
		std::visit([wireframe](auto & mesh) { mesh.Render(wireframe); }, m_mesh_var);
	}

private:
	using mesh_variant_t = std::variant<MeshImpl<PositionVertex>, MeshImpl<NormalVertex>, MeshImpl<TextureVertex>>;

	mesh_variant_t m_mesh_var;
};

template <VertexConcept Vertex>
void MeshImpl<Vertex>::InitBuffers()
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

	if constexpr (VertexSupportsNormal<Vertex>)
	{
		void * normal_offset = reinterpret_cast<void *>(sizeof(glm::vec3));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, normal_offset);
		glEnableVertexAttribArray(1);
	}

	if constexpr (VertexSupportsTexCoord<Vertex>)
	{
		void * tex_offset = reinterpret_cast<void *>(sizeof(glm::vec3) + sizeof(glm::vec3));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, tex_offset);
		glEnableVertexAttribArray(2);
	}
}

template <VertexConcept Vertex>
void MeshImpl<Vertex>::DeleteBuffers()
{
	glDeleteVertexArrays(1, &m_vao_id);
	glDeleteBuffers(1, &m_vbo_id);
	glDeleteBuffers(1, &m_ebo_id);
	m_vao_id = 0;
	m_vbo_id = 0;
	m_ebo_id = 0;
}

template <VertexConcept Vertex>
void MeshImpl<Vertex>::Render(bool wireframe) const
{
	if (m_vao_id == 0)
		return;

	glBindVertexArray(m_vao_id);

	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GLsizei num_elements = static_cast<GLsizei>(m_indices.size());
	glDrawElements(GL_TRIANGLES, num_elements, GL_UNSIGNED_INT, nullptr);
}
