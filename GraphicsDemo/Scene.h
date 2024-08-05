// Scene.h

#pragma once

class Renderer;
class RenderObject;

class Scene
{
public:
	Scene(Renderer & renderer) : m_renderer(renderer) {}

	void Init();
	void Update(double delta_time);

private:
	std::shared_ptr<RenderObject> load_object(std::filesystem::path const & filepath, size_t shader_id) const;

private:
	Renderer & m_renderer;

	std::shared_ptr<RenderObject> m_sword;
	std::shared_ptr<RenderObject> m_red_gem;
	std::shared_ptr<RenderObject> m_green_gem;
	std::shared_ptr<RenderObject> m_blue_gem;

	float m_timer{ 0.0 };
};
