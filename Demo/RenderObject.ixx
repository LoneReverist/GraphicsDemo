// RenderObject.ixx

module;

#include <string>

export module RenderObject;

import AssetPool;

export class RenderObject
{
public:
	explicit RenderObject(std::string name, AssetId mesh_id, AssetId pipeline_id)
		: m_name(name)
		, m_mesh_id(mesh_id)
		, m_pipeline_id(pipeline_id)
	{}

	void SetMeshId(AssetId mesh_id) { m_mesh_id = mesh_id; }
	void SetPipelineId(AssetId pipeline_id) { m_pipeline_id = pipeline_id; }
	void SetObjectData(void const * data) { m_object_data = data; }

	AssetId GetMeshId() const { return m_mesh_id; }
	AssetId GetPipelineId() const { return m_pipeline_id; }
	void const * GetObjectData() const { return m_object_data; }

private:
	std::string m_name; // for debugging

	AssetId m_mesh_id;
	AssetId m_pipeline_id;

	// Pointer to per-object data that gets passed into shaders, expected to be of type Pipeline::ObjectData
	void const * m_object_data = nullptr;
};
