// LightsManager.ixx

module;

#include <glm/vec3.hpp>

export module LightsManager;

export struct AmbientLight
{
	alignas(16) glm::vec3 color{ 1.0, 1.0, 1.0 };
};

export struct PointLight
{
	alignas(16) glm::vec3 pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 color{ 1.0, 1.0, 1.0 };
	alignas(4) float radius = 0.0f;
};

export struct SpotLight
{
	alignas(16) glm::vec3 pos{ 0.0, 0.0, 0.0 };
	alignas(16) glm::vec3 dir{ 0.0, 0.0, -1.0 };
	alignas(16) glm::vec3 color{ 1.0, 1.0, 1.0 };
	alignas(4) float inner_radius = 0.0;
	alignas(4) float outer_radius = 0.0;
};

export struct LightsUniform
{
	alignas(16) AmbientLight ambient_light;
	alignas(16) PointLight pointlight_1;
	alignas(16) PointLight pointlight_2;
	alignas(16) PointLight pointlight_3;
	alignas(16) SpotLight spotlight;
};

export class LightsManager
{
public:
	void SetAmbientLight(AmbientLight const & light) { m_lights.ambient_light = light; }
	void SetPointLight1(PointLight const & light) { m_lights.pointlight_1 = light; }
	void SetPointLight2(PointLight const & light) { m_lights.pointlight_2 = light; }
	void SetPointLight3(PointLight const & light) { m_lights.pointlight_3 = light; }
	void SetSpotLight(SpotLight const & light) { m_lights.spotlight = light; }

	LightsUniform const & GetLightsUniform() const { return m_lights; }

private:
	LightsUniform m_lights;
};
