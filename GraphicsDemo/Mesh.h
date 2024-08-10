// Mesh.h

#pragma once

#include <glm/vec3.hpp>

class Mesh
{
public:
	struct Vertex {
		glm::vec3 m_pos;
		glm::vec3 m_normal;
		glm::vec3 m_color;
	};

	void InitBuffers();
	void DeleteBuffers();

	void Render(bool wireframe) const;

	void SetVerts(std::vector<Vertex> && verts) { m_verts = verts; }
	void SetIndices(std::vector<unsigned int> && indices) { m_indices = indices; }

private:
	std::vector<Vertex> m_verts;
	std::vector<unsigned int> m_indices;

	unsigned int m_vbo_id{ 0 }; // vertex buffer object
	unsigned int m_ebo_id{ 0 }; // element buffer object
	unsigned int m_vao_id{ 0 }; // vertex array object
};
