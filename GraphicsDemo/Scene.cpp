// Scene.cpp

#include "stdafx.h"
#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

#include "ObjLoader.h"
#include "Renderer.h"

void Scene::Init()
{
	size_t color_shader_id = m_renderer.CreateShaderProgram(
		"../resources/shaders/color_vs.txt",
		"../resources/shaders/color_fs.txt");

	m_sword = load_object("../resources/GameObjects/skullsword.obj", color_shader_id);
	m_red_gem = load_object("../resources/GameObjects/redgem.obj", color_shader_id);
	m_green_gem = load_object("../resources/GameObjects/greengem.obj", color_shader_id);
	m_blue_gem = load_object("../resources/GameObjects/bluegem.obj", color_shader_id);

	m_renderer.SetViewTransform(glm::lookAt(
		glm::vec3(0.0f, -10.0f, 5.0f), // camera pos
		glm::vec3(0.0f, 0.0f, 2.5f), // look at pos
		glm::vec3(0.0, 0.0, 1.0) // up dir
	));

	glm::mat4 & sword_transform = m_sword->ModifyWorldTransform();
	sword_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(std::numbers::pi / 4.0), glm::vec3(0.0, 1.0, 0.0)) * sword_transform;
	sword_transform = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.5f, 3.5f)) * sword_transform;

	glm::mat4 & red_gem_transform = m_red_gem->ModifyWorldTransform();
	red_gem_transform = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 4.0f, 2.0f)) * red_gem_transform;
	red_gem_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(0.0 * std::numbers::pi * 2.0 / 3.0), glm::vec3(0.0, 0.0, 1.0)) * red_gem_transform;

	glm::mat4 & green_gem_transform = m_green_gem->ModifyWorldTransform();
	green_gem_transform = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 4.0f, 2.0f)) * green_gem_transform;
	green_gem_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(1.0 * std::numbers::pi * 2.0 / 3.0), glm::vec3(0.0, 0.0, 1.0)) * green_gem_transform;

	glm::mat4 & blue_gem_transform = m_blue_gem->ModifyWorldTransform();
	blue_gem_transform = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 4.0f, 2.0f)) * blue_gem_transform;
	blue_gem_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(2.0 * std::numbers::pi * 2.0 / 3.0), glm::vec3(0.0, 0.0, 1.0)) * blue_gem_transform;
}

void Scene::Update(double delta_time)
{
	m_timer += static_cast<float>(delta_time);

	glm::vec3 bg_color;
	bg_color.r = std::sin(m_timer) / 2.0f + 0.5f;
	bg_color.g = std::cos(m_timer) / 2.0f + 0.5f;
	bg_color.b = std::tan(m_timer) / 2.0f + 0.5f;
	m_renderer.SetClearColor(bg_color);

	// sword update
	glm::mat4 & sword_transform = m_sword->ModifyWorldTransform();

	float sword_pos_z;
	sword_pos_z = std::cos(m_timer * 0.5f) * 0.5f + 3.5f;
	sword_transform[3][2] = sword_pos_z;

	glm::vec3 sword_pos(sword_transform[3][0], sword_transform[3][1], sword_transform[3][2]);
	glm::vec3 sword_y_axis(sword_transform[1][0], sword_transform[1][1], sword_transform[1][2]);
	sword_transform = glm::translate(glm::mat4(1.0), -sword_pos) * sword_transform;
	sword_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(delta_time) * 0.5f, sword_y_axis) * sword_transform;
	sword_transform = glm::translate(glm::mat4(1.0), sword_pos) * sword_transform;

	// red gem update
	glm::mat4 & red_gem_transform = m_red_gem->ModifyWorldTransform();
	red_gem_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(delta_time) * 0.5f, glm::vec3(0.0, 0.0, 1.0)) * red_gem_transform;

	// green gem update
	glm::mat4 & green_gem_transform = m_green_gem->ModifyWorldTransform();
	green_gem_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(delta_time) * 0.5f, glm::vec3(0.0, 0.0, 1.0)) * green_gem_transform;

	// blue gem update
	glm::mat4 & blue_gem_transform = m_blue_gem->ModifyWorldTransform();
	blue_gem_transform = glm::rotate(glm::mat4(1.0), static_cast<float>(delta_time) * 0.5f, glm::vec3(0.0, 0.0, 1.0)) * blue_gem_transform;
}

std::shared_ptr<RenderObject> Scene::load_object(std::filesystem::path const & filepath, size_t shader_id) const
{
	auto obj = std::make_shared<RenderObject>();
	if (!ObjLoader::LoadObjFile(filepath, *obj))
	{
		std::cout << "Scene::load_object() error loading file:" << filepath << std::endl;
		return nullptr;
	}

	obj->InitBuffers();
	obj->SetShaderId(shader_id);
	m_renderer.AddRenderObject(obj);

	// these objects were designed for the Y axis being the up direction, but we're using the Z axis
	glm::mat4 & transform = obj->ModifyWorldTransform();
	transform = glm::rotate(transform, static_cast<float>(std::numbers::pi / 2.0), glm::vec3(1.0, 0.0, 0.0));

	return obj;
}
