// RenderObject.ixx

module;

#include <string>

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

export module RenderObject;

export class RenderObject
{
public:
	RenderObject(std::string name, int mesh_id, int pipeline_id)
		: m_name(name)
		, m_mesh_id(mesh_id)
		, m_pipeline_id(pipeline_id)
	{}

	void SetMeshId(int mesh_id) { m_mesh_id = mesh_id; }
	void SetPipelineId(int pipeline_id) { m_pipeline_id = pipeline_id; }
	void SetObjectData(void const * data) { m_object_data = data; }

	int GetMeshId() const { return m_mesh_id; }
	int GetPipelineId() const { return m_pipeline_id; }
	void const * GetObjectData() const { return m_object_data; }

private:
	std::string m_name; // for debugging

	int m_mesh_id = -1;
	int m_pipeline_id = -1;

	// Pointer to per-object data that gets passed into shaders, expected to be of type CustomPipeline::ObjectData
	void const * m_object_data = nullptr;
};
