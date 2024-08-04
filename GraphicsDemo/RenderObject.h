// RenderObject.h

#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ShaderProgram;

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

	void SetShaderProgram(std::shared_ptr<ShaderProgram> shader_program) { m_shader_program = shader_program; }
	void SetDrawWireframe(bool wireframe = true) { m_draw_wireframe = wireframe; }

	std::shared_ptr<ShaderProgram> GetShaderProgram() const { return m_shader_program; }

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

	std::shared_ptr<ShaderProgram> m_shader_program;

	bool m_draw_wireframe{ false };

	glm::mat4 m_world_transform{ glm::mat4(1.0) };
};
