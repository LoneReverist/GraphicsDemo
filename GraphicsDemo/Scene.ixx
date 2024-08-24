// Scene.ixx

export module Scene;

import <memory>;

import Renderer;
import RenderObject;

export class Scene
{
public:
	Scene(Renderer & renderer) : m_renderer(renderer) {}

	void Init();
	void Update(double delta_time);

private:
	std::shared_ptr<RenderObject> create_object(int mesh_id, int shader_id, int tex_id = -1) const;

private:
	Renderer & m_renderer;

	std::shared_ptr<RenderObject> m_sword0;
	std::shared_ptr<RenderObject> m_sword1;
	std::shared_ptr<RenderObject> m_red_gem;
	std::shared_ptr<RenderObject> m_green_gem;
	std::shared_ptr<RenderObject> m_blue_gem;
	std::shared_ptr<RenderObject> m_ground;
	std::shared_ptr<RenderObject> m_skybox;

	float m_timer{ 0.0 };
};
