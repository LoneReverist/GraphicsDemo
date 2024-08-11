// RenderObject.h

#pragma once

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class RenderObject
{
public:
	RenderObject(int mesh_id, int shader_id)
		: m_mesh_id(mesh_id)
		, m_shader_id(shader_id)
		, m_color({ 1.0, 0.0, 0.5 })
	{}

	void SetMeshId(int mesh_id) { m_mesh_id = mesh_id; }
	void SetShaderId(int shader_id) { m_shader_id = shader_id; }
	void SetColor(glm::vec3 const & color) { m_color = color; }
	void SetDrawWireframe(bool wireframe = true) { m_draw_wireframe = wireframe; }

	int GetMeshId() const { return m_mesh_id; }
	int GetShaderId() const { return m_shader_id; }
	glm::vec3 const & GetColor() { return m_color; }
	bool GetDrawWireframe() const { return m_draw_wireframe; }

	glm::mat4 & ModifyWorldTransform() { return m_world_transform; }
	glm::mat4 const & GetWorldTransform() const { return m_world_transform; }

private:
	int m_mesh_id{ -1 };
	int m_shader_id{ -1 };

	glm::vec3 m_color;

	bool m_draw_wireframe{ false };

	glm::mat4 m_world_transform{ glm::mat4(1.0) };
};
