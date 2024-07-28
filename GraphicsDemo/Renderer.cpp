// Renderer.cpp

#include "stdafx.h"
#include "Renderer.h"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include "ShaderProgram.h"

void Renderer::Init()
{
	m_view_transform = glm::lookAt(
		glm::vec3(0.0f, -2.0f, 2.0f), // camera pos
		glm::vec3(0.0f, 0.0f, 0.0f), // look at pos
		glm::vec3(0.0, 0.0, 1.0)); // up dir

	m_shader_program = std::make_shared<ShaderProgram>();
	m_shader_program->LoadShaders(
		"../resources/shaders/color_vs.txt",
		"../resources/shaders/color_fs.txt");

	m_render_object.Init();
	m_render_object.SetShaderProgram(m_shader_program);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::Update(double delta_time)
{
	static double bg_timer{ 0.0 };
	bg_timer += delta_time;
	m_bg_color.r = static_cast<float>(sin(bg_timer) / 2.0 + 0.5);
	m_bg_color.g = static_cast<float>(cos(bg_timer) / 2.0 + 0.5);
	m_bg_color.b = static_cast<float>(tan(bg_timer) / 2.0 + 0.5);

	glm::mat4 & transform = m_render_object.ModifyWorldTransform();
	transform = glm::rotate(transform, static_cast<float>(delta_time), glm::vec3(0.0, 0.0, 1.0));
}

void Renderer::Render() const
{
	glClearColor(m_bg_color.r, m_bg_color.g, m_bg_color.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::shared_ptr<ShaderProgram> shader_program = m_render_object.GetShaderProgram();
	if (!m_shader_program)
	{
		std::cout << "Renderer::Render() render object has invalid shader program" << std::endl;
		return;
	}

	shader_program->Activate();
	shader_program->SetMat4("world_transform", m_render_object.GetWorldTransform());
	shader_program->SetMat4("view_transform", m_view_transform);
	shader_program->SetMat4("proj_transform", m_proj_transform);

	m_render_object.Render();
}

void Renderer::ResizeViewport(int width, int height)
{
	glViewport(0, 0, width, height);

	constexpr float field_of_view = glm::radians(45.0f);
	const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	const float near_plane = 0.1f;
	const float far_plane = 100.0f;
	m_proj_transform = glm::perspective(field_of_view, aspect_ratio, near_plane, far_plane);
}
