// Scene.cpp

#include "stdafx.h"
#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

#include "ObjLoader.h"
#include "Renderer.h"

namespace
{
	void init_sword_transform(int index, glm::mat4 & transform)
	{
		float x_rot = static_cast<float>(std::numbers::pi / 2.0);
		float y_rot = static_cast<float>(std::numbers::pi / 4.0);
		glm::vec3 pos(0.0f, 0.5f, 3.5f);
		if (index == 1)
		{
			y_rot = -y_rot;
			pos.y = -pos.y;
		}

		transform = glm::rotate(glm::mat4(1.0), x_rot, glm::vec3(1.0, 0.0, 0.0)) * transform;
		transform = glm::rotate(glm::mat4(1.0), y_rot, glm::vec3(0.0, 1.0, 0.0)) * transform;
		transform = glm::translate(glm::mat4(1.0), pos) * transform;
	}

	void update_sword_transform(int index, glm::mat4 & transform, float time, float delta_time)
	{
		float sword_pos_z;
		if (index == 0)
			sword_pos_z = std::cos(time * 0.5f) * 0.5f + 3.5f;
		else
			sword_pos_z = std::sin(time * 0.5f) * 0.5f + 3.5f;
		transform[3][2] = sword_pos_z;

		glm::vec3 sword_pos(transform[3][0], transform[3][1], transform[3][2]);
		glm::vec3 sword_y_axis(transform[1][0], transform[1][1], transform[1][2]);

		transform = glm::translate(glm::mat4(1.0), -sword_pos) * transform;
		transform = glm::rotate(glm::mat4(1.0), delta_time * 0.5f, sword_y_axis) * transform;
		transform = glm::translate(glm::mat4(1.0), sword_pos) * transform;
	}

	void init_gem_transform(int index, glm::mat4 & transform)
	{
		float x_rot = static_cast<float>(std::numbers::pi / 2.0);
		float z_rot = static_cast<float>(index * std::numbers::pi * 2.0 / 3.0);
		glm::vec3 pos(0.0f, 4.0f, 2.0f);

		transform = glm::rotate(glm::mat4(1.0), x_rot, glm::vec3(1.0, 0.0, 0.0)) * transform;
		transform = glm::translate(glm::mat4(1.0), pos) * transform;
		transform = glm::rotate(glm::mat4(1.0), z_rot, glm::vec3(0.0, 0.0, 1.0)) * transform;
	}

	void update_gem_transform(glm::mat4 & transform, float delta_time)
	{
		transform = glm::rotate(glm::mat4(1.0), delta_time * 0.5f, glm::vec3(0.0, 0.0, 1.0)) * transform;
	}
}

void Scene::Init()
{
	int color_shader_id = m_renderer.LoadShaderProgram(
		"../resources/shaders/color_vs.txt",
		"../resources/shaders/color_fs.txt");

	// these meshes were designed for the Y axis being the up direction, but we're using the Z axis, this can be corrected by rotating on the X axis
	int sword_mesh = m_renderer.LoadMesh("../resources/GameObjects/skullsword.obj");
	int red_gem_mesh = m_renderer.LoadMesh("../resources/GameObjects/redgem.obj");
	int green_gem_mesh = m_renderer.LoadMesh("../resources/GameObjects/greengem.obj");
	int blue_gem_mesh = m_renderer.LoadMesh("../resources/GameObjects/bluegem.obj");

	m_sword0 = create_object(sword_mesh, color_shader_id);
	m_sword1 = create_object(sword_mesh, color_shader_id);
	m_red_gem = create_object(red_gem_mesh, color_shader_id);
	m_green_gem = create_object(green_gem_mesh, color_shader_id);
	m_blue_gem = create_object(blue_gem_mesh, color_shader_id);

	m_renderer.SetViewTransform(glm::lookAt(
		glm::vec3(0.0f, -10.0f, 5.0f), // camera pos
		glm::vec3(0.0f, 0.0f, 2.5f), // look at pos
		glm::vec3(0.0, 0.0, 1.0))); // up dir

	init_sword_transform(0, m_sword0->ModifyWorldTransform());
	init_sword_transform(1, m_sword1->ModifyWorldTransform());
	init_gem_transform(0, m_red_gem->ModifyWorldTransform());
	init_gem_transform(1, m_green_gem->ModifyWorldTransform());
	init_gem_transform(2, m_blue_gem->ModifyWorldTransform());
}

void Scene::Update(double delta_time)
{
	float dt = static_cast<float>(delta_time);
	m_timer += dt;

	glm::vec3 bg_color;
	bg_color.r = std::sin(m_timer) / 2.0f + 0.5f;
	bg_color.g = std::cos(m_timer) / 2.0f + 0.5f;
	bg_color.b = std::tan(m_timer) / 2.0f + 0.5f;
	m_renderer.SetClearColor(bg_color);

	update_sword_transform(0, m_sword0->ModifyWorldTransform(), m_timer, dt);
	update_sword_transform(1, m_sword1->ModifyWorldTransform(), m_timer, dt);
	update_gem_transform(m_red_gem->ModifyWorldTransform(), dt);
	update_gem_transform(m_green_gem->ModifyWorldTransform(), dt);
	update_gem_transform(m_blue_gem->ModifyWorldTransform(), dt);
}

std::shared_ptr<RenderObject> Scene::create_object(int mesh_id, int shader_id) const
{
	auto obj = std::make_shared<RenderObject>(mesh_id, shader_id);
	m_renderer.AddRenderObject(obj);
	return obj;
}
