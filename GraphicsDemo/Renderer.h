// Renderer.h

#pragma once

#include <glm/vec3.hpp>

#include "Mesh.h"
#include "RenderObject.h"
#include "ShaderProgram.h"

struct PointLight
{
	glm::vec3 pos{ 0.0, 0.0, 0.0 };
	glm::vec3 color{ 1.0, 1.0, 1.0 };
	float radius{ 0.0f };
};

struct SpotLight
{
	glm::vec3 pos{ 0.0, 0.0, 0.0 };
	glm::vec3 dir{ 0.0, 0.0, -1.0 };
	glm::vec3 color{ 1.0, 1.0, 1.0 };
	float inner_radius{ 0.0 };
	float outer_radius{ 0.0 };
};

class Renderer
{
public:
	~Renderer();

	void Init();
	void Render() const;

	void ResizeViewport(int width, int height);

	void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }
	void SetViewTransform(glm::mat4 const & view_transform) { m_view_transform = view_transform; }
	void SetAmbientLightColor(glm::vec3 const & color) { m_ambient_light_color = color; }

	int LoadShaderProgram(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path);
	int LoadMesh(std::filesystem::path const & mesh_path);
	int AddMesh(Mesh && mesh);
	void AddRenderObject(std::weak_ptr<RenderObject> render_object);

	void SetPointLight1(PointLight const & light) { m_pointlight_1 = light; }
	void SetPointLight2(PointLight const & light) { m_pointlight_2 = light; }
	void SetPointLight3(PointLight const & light) { m_pointlight_3 = light; }
	void SetSpotLight(SpotLight const & light) { m_spotlight = light; }

private:
	glm::vec3 m_clear_color;

	glm::mat4 m_view_transform;
	glm::mat4 m_proj_transform;

	std::vector<ShaderProgram> m_shader_programs;
	std::vector<Mesh> m_meshes;
	std::vector<std::weak_ptr<RenderObject>> m_render_objects;

	glm::vec3 m_ambient_light_color;
	PointLight m_pointlight_1;
	PointLight m_pointlight_2;
	PointLight m_pointlight_3;
	SpotLight m_spotlight;
};
