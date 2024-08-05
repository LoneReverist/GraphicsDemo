// RenderObject.h

#pragma once

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class RenderObject
{
public:
	struct Vertex {
		glm::vec3 m_pos;
		glm::vec3 m_normal;
		glm::vec3 m_color;
	};

	~RenderObject();

	void InitBuffers();
	void Render() const;

	void SetShaderId(size_t shader_id) { m_shader_id = shader_id; }
	void SetDrawWireframe(bool wireframe = true) { m_draw_wireframe = wireframe; }

	size_t GetShaderId() const { return m_shader_id; }

	glm::mat4 & ModifyWorldTransform() { return m_world_transform; }
	glm::mat4 const & GetWorldTransform() const { return m_world_transform; }

	void SetVerts(std::vector<Vertex> && verts) { m_verts = verts; }
	void SetIndices(std::vector<unsigned int> && indices) { m_indices = indices; }

	void LoadSquare();

private:
	std::vector<Vertex> m_verts;
	std::vector<unsigned int> m_indices;

	unsigned int m_vbo_id{ 0 }; // vertex buffer object
	unsigned int m_ebo_id{ 0 }; // element buffer object
	unsigned int m_vao_id{ 0 }; // vertex array object

	size_t m_shader_id{ 0 };

	bool m_draw_wireframe{ false };

	glm::mat4 m_world_transform{ glm::mat4(1.0) };
};
