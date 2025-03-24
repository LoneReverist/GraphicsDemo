// Scene.ixx

module;

#include <glm/vec3.hpp>

export module Scene;

import <memory>;

import Input;
import Renderer;
import RenderObject;
import Texture;

export class Scene
{
public:
	Scene(Renderer & renderer) : m_renderer(renderer) {}

	void Init();
	void Update(double delta_time, Input const & input);

private:
	std::shared_ptr<RenderObject> create_object(int mesh_id, int pipeline_id, int tex_id = -1) const;

	void update_camera(float dt, Input const & input);

private:
	Renderer & m_renderer;

	std::unique_ptr<Texture> m_ground_tex;

	//std::shared_ptr<RenderObject> m_sword0;
	//std::shared_ptr<RenderObject> m_sword1;
	std::shared_ptr<RenderObject> m_red_gem;
	std::shared_ptr<RenderObject> m_green_gem;
	std::shared_ptr<RenderObject> m_blue_gem;
	std::shared_ptr<RenderObject> m_ground;
	//std::shared_ptr<RenderObject> m_skybox;

	float m_timer{ 0.0 };

	glm::vec3 m_camera_pos;
	glm::vec3 m_camera_dir;
};
