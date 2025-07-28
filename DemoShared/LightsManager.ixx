// LightsManager.ixx

module;

#include <glm/vec3.hpp>

export module LightsManager;

export struct AmbientLight
{
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
};

export struct PointLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_radius = 0.0f;
};

export struct SpotLight
{
	alignas(16) glm::vec3 m_pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 m_dir{ 0.0, 0.0, -1.0 };
	alignas(16) glm::vec3 m_color{ 1.0, 1.0, 1.0 };
	alignas(4) float m_inner_radius = 0.0;
	alignas(4) float m_outer_radius = 0.0;
};

export struct LightsUniform
{
	alignas(16) AmbientLight m_ambient_light;
	alignas(16) PointLight m_pointlight_1;
	alignas(16) PointLight m_pointlight_2;
	alignas(16) PointLight m_pointlight_3;
	alignas(16) SpotLight m_spotlight;
};

export class LightsManager
{
public:
	void SetAmbientLight(AmbientLight const & light) { m_lights.m_ambient_light = light; }
	void SetPointLight1(PointLight const & light) { m_lights.m_pointlight_1 = light; }
	void SetPointLight2(PointLight const & light) { m_lights.m_pointlight_2 = light; }
	void SetPointLight3(PointLight const & light) { m_lights.m_pointlight_3 = light; }
	void SetSpotLight(SpotLight const & light) { m_lights.m_spotlight = light; }

	LightsUniform const & GetLightsUniform() const { return m_lights; }

private:
	LightsUniform m_lights;
};
