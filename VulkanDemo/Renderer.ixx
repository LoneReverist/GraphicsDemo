// Renderer.ixx

module;

#include <filesystem>

#include <vulkan/vulkan.h>

#include <glm/vec3.hpp>

export module Renderer;

import GraphicsApi;
import GraphicsPipeline;
import Mesh;
import RenderObject;

struct PipelineContainer
{
	std::unique_ptr<GraphicsPipeline> m_pipeline;
	std::vector<std::weak_ptr<RenderObject>> m_render_objects;
};

export class Renderer
{
public:
	explicit Renderer(GraphicsApi const & graphics_api);

	void Render() const;

	int AddGraphicsPipeline(std::unique_ptr<GraphicsPipeline> pipeline);

	int LoadMesh(std::filesystem::path const & mesh_path);
	int AddMesh(Mesh && mesh_var);

	std::shared_ptr<RenderObject> CreateRenderObject(std::string name, int mesh_id, int pipeline_id);

	void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

private:
	GraphicsApi const & m_graphics_api;

	std::vector<PipelineContainer> m_pipeline_containers;

	std::vector<Mesh> m_meshes; // TODO: need asset manager

	glm::vec3 m_clear_color;
};
