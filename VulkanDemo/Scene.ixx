// Scene.ixx

module;

#include <memory>

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

export module Scene;

import Camera;
import GraphicsApi;
import Input;
import Renderer;
import RenderObject;
import Texture;

struct PointLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_radius{ 0.0f };
};

struct SpotLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_dir{ 0.0, 0.0, -1.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_inner_radius{ 0.0 };
	alignas(4) float m_outer_radius{ 0.0 };
};

export class Scene
{
public:
	explicit Scene(GraphicsApi const & graphics_api)
		: m_graphics_api{ graphics_api }
		, m_renderer{ graphics_api }
	{}

	void Init();
	void OnViewportResized(int width, int height);

	void Update(double delta_time, Input const & input);
	void Render() const;

	GraphicsApi const & GetGraphicsApi() const { return m_graphics_api; }
	Renderer const & GetRenderer() const { return m_renderer; }

	glm::vec3 const & GetAmbientLightColor() const { return m_ambient_light_color; }
	PointLight const & GetPointLight1() const { return m_pointlight_1; }
	PointLight const & GetPointLight2() const { return m_pointlight_2; }
	PointLight const & GetPointLight3() const { return m_pointlight_3; }
	SpotLight const & GetSpotLight() const { return m_spotlight; }

	Camera const & GetCamera() const { return m_camera; }

private:
	GraphicsApi const & m_graphics_api;

	Renderer m_renderer;
	Camera m_camera;

	std::unique_ptr<Texture> m_ground_tex;
	std::unique_ptr<Texture> m_skybox_tex;

	std::shared_ptr<RenderObject> m_sword0;
	std::shared_ptr<RenderObject> m_sword1;
	std::shared_ptr<RenderObject> m_red_gem;
	std::shared_ptr<RenderObject> m_green_gem;
	std::shared_ptr<RenderObject> m_blue_gem;
	std::shared_ptr<RenderObject> m_ground;
	std::shared_ptr<RenderObject> m_skybox;

	glm::vec3 m_ambient_light_color;
	PointLight m_pointlight_1;
	PointLight m_pointlight_2;
	PointLight m_pointlight_3;
	SpotLight m_spotlight;

	float m_timer{ 0.0 };
};
