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

		// vertex shader uniforms
		shader_program.SetUniform("world_transform", obj->GetWorldTransform());
		shader_program.SetUniform("view_transform", m_view_transform);
		shader_program.SetUniform("proj_transform", m_proj_transform);

		// fragment shader uniforms
		shader_program.SetUniform("object_color", obj->GetColor());

		shader_program.SetUniform("ambient_light_color", m_ambient_light_color);
		shader_program.SetUniform("pointlight_pos_1", m_pointlight_1.m_pos);
		shader_program.SetUniform("pointlight_color_1", m_pointlight_1.m_color);
		shader_program.SetUniform("pointlight_radius_1", m_pointlight_1.m_radius);
		shader_program.SetUniform("pointlight_pos_2", m_pointlight_2.m_pos);
		shader_program.SetUniform("pointlight_color_2", m_pointlight_2.m_color);
		shader_program.SetUniform("pointlight_radius_2", m_pointlight_2.m_radius);
		shader_program.SetUniform("pointlight_pos_3", m_pointlight_3.m_pos);
		shader_program.SetUniform("pointlight_color_3", m_pointlight_3.m_color);
		shader_program.SetUniform("pointlight_radius_3", m_pointlight_3.m_radius);
		shader_program.SetUniform("spotlight_pos", m_spotlight.m_pos);
		shader_program.SetUniform("spotlight_dir", m_spotlight.m_dir);
		shader_program.SetUniform("spotlight_color", m_spotlight.m_color);
		shader_program.SetUniform("spotlight_inner_radius", m_spotlight.m_inner_radius);
		shader_program.SetUniform("spotlight_outer_radius", m_spotlight.m_outer_radius);

		shader_program.SetUniform("camera_pos_world", m_camera_pos);

		int tex_id = obj->GetTextureId();
		if (tex_id != -1)
		{
			Texture const & texture = m_textures[tex_id];
			texture.Bind();
		}

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
	std::vector<BasicVertex> vertices;
	std::vector<unsigned int> indices;
	if (!ObjLoader::LoadObjFile(mesh_path, vertices, indices))
	{
		std::cout << "Renderer::LoadMesh() error loading file:" << mesh_path << std::endl;
		return -1;
	}

	Mesh mesh{ std::move(vertices), std::move(indices) };
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

int Renderer::LoadTexture(std::filesystem::path const & tex_path)
{
	Texture texture;
	if (!texture.LoadTexture(tex_path))
		return -1;

	m_textures.push_back(std::move(texture));
	return static_cast<int>(m_textures.size());
}

void Renderer::AddRenderObject(std::weak_ptr<RenderObject> render_object)
{
	m_render_objects.push_back(render_object);
}

void Renderer::SetCamera(glm::vec3 const & pos, glm::vec3 const & look_at_pos)
{
	m_camera_pos = pos;
	m_view_transform = glm::lookAt(pos, look_at_pos, glm::vec3(0.0, 0.0, 1.0));
}
