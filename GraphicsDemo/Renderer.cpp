// Renderer.cpp

#include "stdafx.h"
#include "Renderer.h"

#include <glad/glad.h>

#include "ObjLoader.h"

namespace
{
	std::string type_to_string(GLenum type)
	{
		switch (type) {
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		default: return "UNKNOWN";
		}
	}

	std::string severity_to_string(GLenum severity)
	{
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		default: return "UNKNOWN";
		}
	}

	void GLAPIENTRY debug_message_callback(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar * message,
		const void * userParam)
	{
		if (id == 131185) // ignore notification about using GL_STATIC_DRAW
			return;

		std::cout << std::format("OpenGL {3}: type - {1}, id - {2}\nMessage: {0}\n\n",
			message, type_to_string(type), id, severity_to_string(severity));
	}
}

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
	glEnable(GL_CULL_FACE); // cull back facing facets. by default, front facing facets have counter-clockwise vertex windings.

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debug_message_callback, 0);
}

void Renderer::render_skybox() const
{
	std::shared_ptr<RenderObject> skybox = m_skybox.lock();
	if (!skybox)
		return;

	int mesh_id = skybox->GetMeshId();
	int shader_id = skybox->GetShaderId();
	int tex_id = skybox->GetTextureId();
	if (mesh_id == -1 || shader_id == -1 || tex_id == -1)
		return;

	glDepthMask(GL_FALSE);

	ShaderProgram const & shader_program = m_shader_programs[shader_id];
	shader_program.Activate();

	// vertex shader uniforms
	glm::mat4 view_transform = glm::mat4(glm::mat3(m_view_transform)); // drop the translation
	shader_program.SetUniform("view_transform", view_transform);
	shader_program.SetUniform("proj_transform", m_proj_transform);

	Texture const & texture = m_textures[tex_id];
	texture.Bind();

	Mesh const & mesh = m_meshes[mesh_id];
	mesh.Render(skybox->GetDrawWireframe());

	glDepthMask(GL_TRUE);
}

void Renderer::Render() const
{
	glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_skybox();

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
	std::vector<NormalVertex> vertices;
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
	return static_cast<int>(m_textures.size() - 1);
}

int Renderer::LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths)
{
	Texture texture;
	if (!texture.LoadCubeMap(filepaths))
		return -1;

	m_textures.push_back(std::move(texture));
	return static_cast<int>(m_textures.size() - 1);
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
