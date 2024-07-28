// Renderer.cpp

#include "stdafx.h"
#include "Renderer.h"

#include <glad/glad.h>

#include "ShaderProgram.h"

void Renderer::Init()
{
	m_shader_program = std::make_shared<ShaderProgram>();
	m_shader_program->LoadShaders(
		"../resources/shaders/color_vs.txt",
		"../resources/shaders/color_fs.txt");

	m_render_object.Init();
	m_render_object.SetShaderProgram(m_shader_program);
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
	glClear(GL_COLOR_BUFFER_BIT);

	m_render_object.Render();
}

void Renderer::ResizeViewport(int width, int height) const
{
	glViewport(0, 0, width, height);
}
