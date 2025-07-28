// Renderer.ixx

module;

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec3.hpp>

export module Renderer;

import GraphicsApi;
import GraphicsPipeline;
import Mesh;
import RenderObject;

struct PipelineContainer
{
	GraphicsPipeline m_pipeline;
	std::vector<std::weak_ptr<RenderObject>> m_render_objects;
};

export class Renderer
{
public:
	explicit Renderer(GraphicsApi const & graphics_api);

	void Render() const;

	int AddPipeline(GraphicsPipeline && pipeline);
	int AddMesh(Mesh && mesh);
	void UpdateMesh(int index, Mesh && mesh);

	std::shared_ptr<RenderObject> CreateRenderObject(std::string const & name, int mesh_id, int pipeline_id);

	void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

private:
	GraphicsApi const & m_graphics_api;

	std::vector<PipelineContainer> m_pipeline_containers;

	std::vector<Mesh> m_meshes; // TODO: need asset manager

	glm::vec3 m_clear_color;
};
