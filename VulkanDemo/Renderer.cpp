// Renderer.cpp

module;

#include <iostream>

#include <vulkan/vulkan.h>

#include <glm/gtc/matrix_transform.hpp>

module Renderer;

//import <iostream>;

//import ObjLoader;

Renderer::Renderer(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

Renderer::~Renderer()
{
	for (GraphicsPipeline & pipeline : m_pipelines)
		pipeline.DestroyPipeline();
//	for (Mesh & mesh : m_meshes)
//		mesh.DeleteBuffers();
}

void Renderer::Init()
{
	m_view_transform = glm::mat4(1.0);

//	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_CULL_FACE); // cull back facing facets. by default, front facing facets have counter-clockwise vertex windings.
}

//void Renderer::render_skybox() const
//{
//	std::shared_ptr<RenderObject> skybox = m_skybox.lock();
//	if (!skybox)
//		return;
//
//	int mesh_id = skybox->GetMeshId();
//	int shader_id = skybox->GetShaderId();
//	int tex_id = skybox->GetTextureId();
//	if (mesh_id == -1 || shader_id == -1 || tex_id == -1)
//		return;
//
//	glDepthMask(GL_FALSE);
//
//	ShaderProgram const & shader_program = m_shader_programs[shader_id];
//	shader_program.Activate();
//
//	// vertex shader uniforms
//	glm::mat4 view_transform = glm::mat4(glm::mat3(m_view_transform)); // drop the translation
//	shader_program.SetUniform("view_transform", view_transform);
//	shader_program.SetUniform("proj_transform", m_proj_transform);
//
//	Texture const & texture = m_textures[tex_id];
//	texture.Bind();
//
//	Mesh const & mesh = m_meshes[mesh_id];
//	mesh.Render(skybox->GetDrawWireframe());
//
//	glDepthMask(GL_TRUE);
//}

void Renderer::Render() const
{
	VkCommandBuffer command_buffer = m_graphics_api.GetCommandBuffer();
	VkExtent2D sc_extent = m_graphics_api.GetSwapChainExtent();

	VkCommandBufferBeginInfo begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};

	VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to begin recording command buffer!");

	VkClearValue clear_color = { .color = { m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f } };

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(sc_extent.width),
		.height = static_cast<float>(sc_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor{
		.offset = { 0, 0 },
		.extent = sc_extent
	};

	VkRenderPassBeginInfo render_pass_info{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_graphics_api.GetRenderPass(),
		.framebuffer = m_graphics_api.GetCurFrameBuffer(),
		.renderArea = scissor,
		.clearValueCount = 1,
		.pClearValues = &clear_color
	};

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	m_pipelines[0].Activate();

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	vkCmdDraw(command_buffer,
		3 /*vertexCount*/,
		1 /*instanceCount*/,
		0 /*firstVertex*/,
		0 /*firstInstance*/);

	vkCmdEndRenderPass(command_buffer);

	result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer!");

//	glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	render_skybox();
//
//	for (std::weak_ptr<RenderObject> render_object : m_render_objects)
//	{
//		std::shared_ptr<RenderObject> obj = render_object.lock();
//		if (!obj)
//			continue;
//
//		int mesh_id = obj->GetMeshId();
//		int shader_id = obj->GetShaderId();
//		if (mesh_id == -1 || shader_id == -1)
//			continue;
//
//		ShaderProgram const & shader_program = m_shader_programs[shader_id];
//		shader_program.Activate();
//
//		// vertex shader uniforms
//		shader_program.SetUniform("world_transform", obj->GetWorldTransform());
//		shader_program.SetUniform("view_transform", m_view_transform);
//		shader_program.SetUniform("proj_transform", m_proj_transform);
//
//		// fragment shader uniforms
//		shader_program.SetUniform("object_color", obj->GetColor());
//
//		shader_program.SetUniform("ambient_light_color", m_ambient_light_color);
//		shader_program.SetUniform("pointlight_1.pos", m_pointlight_1.m_pos);
//		shader_program.SetUniform("pointlight_1.color", m_pointlight_1.m_color);
//		shader_program.SetUniform("pointlight_1.radius", m_pointlight_1.m_radius);
//		shader_program.SetUniform("pointlight_2.pos", m_pointlight_2.m_pos);
//		shader_program.SetUniform("pointlight_2.color", m_pointlight_2.m_color);
//		shader_program.SetUniform("pointlight_2.radius", m_pointlight_2.m_radius);
//		shader_program.SetUniform("pointlight_3.pos", m_pointlight_3.m_pos);
//		shader_program.SetUniform("pointlight_3.color", m_pointlight_3.m_color);
//		shader_program.SetUniform("pointlight_3.radius", m_pointlight_3.m_radius);
//		shader_program.SetUniform("spotlight_1.pos", m_spotlight.m_pos);
//		shader_program.SetUniform("spotlight_1.dir", m_spotlight.m_dir);
//		shader_program.SetUniform("spotlight_1.color", m_spotlight.m_color);
//		shader_program.SetUniform("spotlight_1.inner_radius", m_spotlight.m_inner_radius);
//		shader_program.SetUniform("spotlight_1.outer_radius", m_spotlight.m_outer_radius);
//
//		shader_program.SetUniform("camera_pos_world", m_camera_pos);
//
//		int tex_id = obj->GetTextureId();
//		if (tex_id != -1)
//		{
//			Texture const & texture = m_textures[tex_id];
//			texture.Bind();
//		}
//
//		Mesh const & mesh = m_meshes[mesh_id];
//		mesh.Render(obj->GetDrawWireframe());
//	}
}

void Renderer::ResizeViewport(int width, int height)
{
//	glViewport(0, 0, width, height);
//
//	constexpr float field_of_view = glm::radians(45.0f);
//	const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
//	const float near_plane = 0.1f;
//	const float far_plane = 100.0f;
//	m_proj_transform = glm::perspective(field_of_view, aspect_ratio, near_plane, far_plane);
}

void Renderer::LoadGraphicsPipeline(
	std::filesystem::path const & vert_shader_path,
	std::filesystem::path const & frag_shader_path)
{
	m_pipelines.emplace_back(m_graphics_api);

	if (!m_pipelines.back().CreatePipeline(vert_shader_path, frag_shader_path))
		m_pipelines.pop_back();
}

//int Renderer::LoadShaderProgram(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path)
//{
//	ShaderProgram shader_program;
//	if (!shader_program.LoadShaders(vert_shader_path, frag_shader_path))
//		return -1;
//
//	m_shader_programs.push_back(std::move(shader_program));
//	return static_cast<int>(m_shader_programs.size() - 1);
//}
//
//int Renderer::LoadMesh(std::filesystem::path const & mesh_path)
//{
//	std::vector<NormalVertex> vertices;
//	std::vector<unsigned int> indices;
//	if (!ObjLoader::LoadObjFile(mesh_path, vertices, indices))
//	{
//		std::cout << "Renderer::LoadMesh() error loading file:" << mesh_path << std::endl;
//		return -1;
//	}
//
//	Mesh mesh{ std::move(vertices), std::move(indices) };
//	mesh.InitBuffers();
//
//	m_meshes.push_back(std::move(mesh));
//	return static_cast<int>(m_meshes.size() - 1);
//}
//
//int Renderer::AddMesh(Mesh && mesh)
//{
//	mesh.InitBuffers();
//
//	m_meshes.push_back(std::move(mesh));
//	return static_cast<int>(m_meshes.size() - 1);
//}
//
//int Renderer::LoadTexture(std::filesystem::path const & tex_path)
//{
//	Texture texture;
//	if (!texture.LoadTexture(tex_path))
//		return -1;
//
//	m_textures.push_back(std::move(texture));
//	return static_cast<int>(m_textures.size() - 1);
//}
//
//int Renderer::LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths)
//{
//	Texture texture;
//	if (!texture.LoadCubeMap(filepaths))
//		return -1;
//
//	m_textures.push_back(std::move(texture));
//	return static_cast<int>(m_textures.size() - 1);
//}
//
//void Renderer::AddRenderObject(std::weak_ptr<RenderObject> render_object)
//{
//	m_render_objects.push_back(render_object);
//}

void Renderer::SetCamera(glm::vec3 const & pos, glm::vec3 const & dir)
{
	m_camera_pos = pos;
	m_view_transform = glm::lookAt(pos, pos + dir, glm::vec3(0.0, 0.0, 1.0));
}
