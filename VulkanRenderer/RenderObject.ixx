// RenderObject.ixx

module;

#include <string>

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

export module RenderObject;

export class RenderObject
{
public:
	RenderObject(std::string name, int mesh_id, int pipeline_id, int tex_id = -1)
		: m_name(name)
		, m_mesh_id(mesh_id)
		, m_pipeline_id(pipeline_id)
		, m_tex_id(tex_id)
		, m_color({ 1.0, 1.0, 1.0 })
	{}

	void SetMeshId(int mesh_id) { m_mesh_id = mesh_id; }
	void SetPipelineId(int pipeline_id) { m_pipeline_id = pipeline_id; }
	void SetTextureId(int tex_id) { m_tex_id = tex_id; }
	void SetColor(glm::vec3 const & color) { m_color = color; }
	void SetDrawWireframe(bool wireframe = true) { m_draw_wireframe = wireframe; }

	int GetMeshId() const { return m_mesh_id; }
	int GetPipelineId() const { return m_pipeline_id; }
	int GetTextureId() const { return m_tex_id; }
	glm::vec3 const & GetColor() const { return m_color; }
	bool GetDrawWireframe() const { return m_draw_wireframe; }

	glm::mat4 & ModifyModelTransform() { return m_model_transform; }
	glm::mat4 const & GetModelTransform() const { return m_model_transform; }

private:
	std::string m_name; // for debugging

	int m_mesh_id{ -1 };
	int m_pipeline_id{ -1 };
	int m_tex_id{ -1 };

	glm::vec3 m_color;

	bool m_draw_wireframe{ false };

	glm::mat4 m_model_transform{ 1.0 };
};
