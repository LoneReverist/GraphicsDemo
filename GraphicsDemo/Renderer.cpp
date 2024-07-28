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
	m_bg_color[0] = sin(time) / 2.0f + 0.5f;
	m_bg_color[1] = cos(time) / 2.0f + 0.5f;
	m_bg_color[2] = tan(time) / 2.0f + 0.5f;
}

void Renderer::Render()
{
	glClearColor(m_bg_color[0], m_bg_color[1], m_bg_color[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	m_render_object.Render();
}
