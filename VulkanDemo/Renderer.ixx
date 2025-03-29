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
//import Texture;

export struct PointLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_radius{ 0.0f };
};

export struct SpotLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_dir{ 0.0, 0.0, -1.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_inner_radius{ 0.0 };
	alignas(4) float m_outer_radius{ 0.0 };
};

struct PipelineContainer
{
	std::unique_ptr<GraphicsPipeline> m_pipeline;
	std::vector<std::weak_ptr<RenderObject>> m_render_objects;
};

export class Renderer
{
public:
	Renderer(GraphicsApi const & graphics_api);
	~Renderer();

	void Init(int width, int height);
	void Render() const;

	void OnViewportResized(int width, int height);

	int AddGraphicsPipeline(std::unique_ptr<GraphicsPipeline> pipeline);

	int LoadMesh(std::filesystem::path const & mesh_path);
	int AddMesh(Mesh && mesh_var);

	void AddRenderObject(std::weak_ptr<RenderObject> render_object);

	void SetCamera(glm::vec3 const & pos, glm::vec3 const & dir);

	void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

	void SetAmbientLightColor(glm::vec3 const & color) { m_ambient_light_color = color; }
	void SetPointLight1(PointLight const & light) { m_pointlight_1 = light; }
	void SetPointLight2(PointLight const & light) { m_pointlight_2 = light; }
	void SetPointLight3(PointLight const & light) { m_pointlight_3 = light; }
	void SetSpotLight(SpotLight const & light) { m_spotlight = light; }

	glm::vec3 const & GetAmbientLightColor() const { return m_ambient_light_color; }
	PointLight const & GetPointLight1() const { return m_pointlight_1; }
	PointLight const & GetPointLight2() const { return m_pointlight_2; }
	PointLight const & GetPointLight3() const { return m_pointlight_3; }
	SpotLight const & GetSpotLight() const { return m_spotlight; }

	GraphicsApi const & GetGraphicsApi() const { return m_graphics_api; }
	glm::mat4 const & GetViewTransform() const { return m_view_transform; }
	glm::mat4 const & GetProjTransform() const { return m_proj_transform; }
	glm::vec3 const & GetCameraPos() const { return m_camera_pos; }

private:
	GraphicsApi const & m_graphics_api;

	std::vector<PipelineContainer> m_pipeline_containers;

	std::vector<Mesh> m_meshes; // TODO: need asset manager

	glm::vec3 m_camera_pos; // TODO: need camera class

	glm::mat4 m_view_transform;
	glm::mat4 m_proj_transform;

	glm::vec3 m_clear_color;

	glm::vec3 m_ambient_light_color; // TODO: need light container/manager
	PointLight m_pointlight_1;
	PointLight m_pointlight_2;
	PointLight m_pointlight_3;
	SpotLight m_spotlight;
};
