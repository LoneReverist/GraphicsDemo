// Renderer.cpp

#include "stdafx.h"
#include "Renderer.h"

#include <glad/glad.h>

#include "ObjLoader.h"

Renderer::~Renderer()
{
	for (ShaderProgram & shader_program : m_shader_programs)
		shader_program.DeleteShaders();
	for (Mesh & mesh : m_meshes)
		mesh.DeleteBuffers();
}

void Renderer::Init()
{
	m_view_transform = glm::mat4(1.0);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::Render() const
{
	glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (std::weak_ptr<RenderObject> render_object : m_render_objects)
	{
		std::shared_ptr<RenderObject> obj = render_object.lock();
		if (!obj)
			continue;

		int mesh_id = obj->GetMeshId();
		int shader_id = obj->GetShaderId();
		if (mesh_id == -1 || shader_id == -1)
			continue;

		ShaderProgram const & shader_program = m_shader_programs[shader_id];
		shader_program.Activate();
		shader_program.SetUniform("world_transform", obj->GetWorldTransform());
		shader_program.SetUniform("view_transform", m_view_transform);
		shader_program.SetUniform("proj_transform", m_proj_transform);
		shader_program.SetUniform("mesh_color", obj->GetColor());

		Mesh const & mesh = m_meshes[mesh_id];
		mesh.Render(obj->GetDrawWireframe());
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

int Renderer::LoadShaderProgram(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path)
{
	ShaderProgram shader_program;
	if (!shader_program.LoadShaders(vert_shader_path, frag_shader_path))
		return -1;

	m_shader_programs.push_back(std::move(shader_program));
	return static_cast<int>(m_shader_programs.size() - 1);
}

int Renderer::LoadMesh(std::filesystem::path const & mesh_path)
{
	Mesh mesh;
	if (!ObjLoader::LoadObjFile(mesh_path, mesh))
	{
		std::cout << "Renderer::LoadMesh() error loading file:" << mesh_path << std::endl;
		return -1;
	}

	mesh.InitBuffers();

	m_meshes.push_back(std::move(mesh));
	return static_cast<int>(m_meshes.size() - 1);
}

int Renderer::AddMesh(Mesh && mesh)
{
	mesh.InitBuffers();

	m_meshes.push_back(std::move(mesh));
	return static_cast<int>(m_meshes.size() - 1);
}

void Renderer::AddRenderObject(std::weak_ptr<RenderObject> render_object)
{
	m_render_objects.push_back(render_object);
}
