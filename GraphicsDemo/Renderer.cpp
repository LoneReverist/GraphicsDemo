// Renderer.cpp

#include "stdafx.h"
#include "Renderer.h"

#include <glad/glad.h>

void Renderer::Init()
{
	m_view_transform = glm::mat4(1.0);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::Render() const
{
	glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (std::shared_ptr<RenderObject> render_object : m_render_objects)
	{
		ShaderProgram const & shader_program = m_shader_programs[render_object->GetShaderId()];

		shader_program.Activate();
		shader_program.SetMat4("world_transform", render_object->GetWorldTransform());
		shader_program.SetMat4("view_transform", m_view_transform);
		shader_program.SetMat4("proj_transform", m_proj_transform);

		render_object->Render();
	}
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

size_t Renderer::CreateShaderProgram(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path)
{
	ShaderProgram & shader_program = m_shader_programs.emplace_back();
	shader_program.LoadShaders(vert_shader_path, frag_shader_path);
	return m_shader_programs.size() - 1;
}

void Renderer::AddRenderObject(std::shared_ptr<RenderObject> render_object)
{
	m_render_objects.push_back(render_object);
}
