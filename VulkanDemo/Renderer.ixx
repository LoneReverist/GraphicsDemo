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
	glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	float m_radius{ 0.0f };
};

export struct SpotLight
{
	glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	glm::vec3 m_dir{ 0.0, 0.0, -1.0 };
	glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	float m_inner_radius{ 0.0 };
	float m_outer_radius{ 0.0 };
};

export class Renderer
{
public:
	Renderer(GraphicsApi const & graphics_api);
	~Renderer();

	void Init();
	void Render() const;

	void OnViewportResized(int width, int height);

	int LoadGraphicsPipeline(
		std::filesystem::path const & vert_shader_path,
		std::filesystem::path const & frag_shader_path,
		VkVertexInputBindingDescription const & binding_desc,
		std::vector<VkVertexInputAttributeDescription> const & attrib_descs);

	int LoadMesh(std::filesystem::path const & mesh_path);
//	int AddMesh(Mesh && mesh_var);
//	int LoadTexture(std::filesystem::path const & tex_path);
//	int LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths);

	void AddRenderObject(std::weak_ptr<RenderObject> render_object);
//	void SetSkybox(std::weak_ptr<RenderObject> skybox) { m_skybox = skybox; }

	void SetCamera(glm::vec3 const & pos, glm::vec3 const & dir);

	void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

	void SetAmbientLightColor(glm::vec3 const & color) { m_ambient_light_color = color; }
	void SetPointLight1(PointLight const & light) { m_pointlight_1 = light; }
	void SetPointLight2(PointLight const & light) { m_pointlight_2 = light; }
	void SetPointLight3(PointLight const & light) { m_pointlight_3 = light; }
	void SetSpotLight(SpotLight const & light) { m_spotlight = light; }

private:
//	void render_skybox() const;

private:
	GraphicsApi const & m_graphics_api;

	std::vector<GraphicsPipeline> m_pipelines;
	std::vector<Mesh> m_meshes;
//	std::vector<Texture> m_textures;

	std::vector<std::weak_ptr<RenderObject>> m_render_objects;
//	std::weak_ptr<RenderObject> m_skybox;

	glm::vec3 m_camera_pos;

	glm::mat4 m_view_transform;
	glm::mat4 m_proj_transform;

	glm::vec3 m_clear_color;

	glm::vec3 m_ambient_light_color;
	PointLight m_pointlight_1;
	PointLight m_pointlight_2;
	PointLight m_pointlight_3;
	SpotLight m_spotlight;
};
