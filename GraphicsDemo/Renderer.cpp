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

void Renderer::Update(float time)
{
	m_bg_color.r = sin(time) / 2.0f + 0.5f;
	m_bg_color.g = cos(time) / 2.0f + 0.5f;
	m_bg_color.b = tan(time) / 2.0f + 0.5f;

	glm::mat4 & transform = m_render_object.ModifyWorldTransform();
	transform = glm::rotate(glm::mat4(1.0), time, glm::vec3(0.0, 0.0, 1.0));
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
