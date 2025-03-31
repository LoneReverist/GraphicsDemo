// Renderer.ixx

module;

#include <vulkan/vulkan.h>

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

export module Renderer;

import <filesystem>;

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
	~Renderer();

	void Init(int width, int height);
	void Render() const;

	void OnViewportResized(int width, int height);

	int AddGraphicsPipeline(std::unique_ptr<GraphicsPipeline> pipeline);

	int LoadMesh(std::filesystem::path const & mesh_path);
	int AddMesh(Mesh && mesh_var);

	void AddRenderObject(std::weak_ptr<RenderObject> render_object);

	void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

	GraphicsApi const & GetGraphicsApi() const { return m_graphics_api; }
	glm::mat4 const & GetProjTransform() const { return m_proj_transform; }

private:
	GraphicsApi const & m_graphics_api;

	std::vector<PipelineContainer> m_pipeline_containers;

	std::vector<Mesh> m_meshes; // TODO: need asset manager

	glm::mat4 m_proj_transform;

	glm::vec3 m_clear_color;
};
