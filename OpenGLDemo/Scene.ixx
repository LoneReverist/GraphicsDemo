// Scene.ixx

module;

#include <memory>
#include <vector>

#include <glm/vec3.hpp>

export module Scene;

import Camera;
import FontAtlas;
import Input;
import Renderer;
import RenderObject;
import Texture;

struct AmbientLight
{
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
};

struct PointLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_radius = 0.0f;
};

struct SpotLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_dir{ 0.0, 0.0, -1.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_inner_radius = 0.0;
	alignas(4) float m_outer_radius = 0.0;
};

export class Scene
{
public:
	Scene() = default;

	void Init();
	void OnViewportResized(int width, int height);

	void Update(double delta_time, Input const & input);
	void Render() const;

	Renderer const & GetRenderer() const { return m_renderer; }
	Camera const & GetCamera() const { return m_camera; }

	AmbientLight const & GetAmbientLight() const { return m_ambient_light; }
	PointLight const & GetPointLight1() const { return m_pointlight_1; }
	PointLight const & GetPointLight2() const { return m_pointlight_2; }
	PointLight const & GetPointLight3() const { return m_pointlight_3; }
	SpotLight const & GetSpotLight() const { return m_spotlight; }

	Texture const & GetTexture(int id) const;

private:
	Renderer m_renderer;
	Camera m_camera;

	std::vector<std::unique_ptr<Texture>> m_textures;
	std::unique_ptr<FontAtlas> m_arial_font;

	std::shared_ptr<RenderObject> m_sword0;
	std::shared_ptr<RenderObject> m_sword1;
	std::shared_ptr<RenderObject> m_red_gem;
	std::shared_ptr<RenderObject> m_green_gem;
	std::shared_ptr<RenderObject> m_blue_gem;
	std::shared_ptr<RenderObject> m_ground;
	std::shared_ptr<RenderObject> m_skybox;

	AmbientLight m_ambient_light;
	PointLight m_pointlight_1;
	PointLight m_pointlight_2;
	PointLight m_pointlight_3;
	SpotLight m_spotlight;

	float m_timer = 0.0;
};
