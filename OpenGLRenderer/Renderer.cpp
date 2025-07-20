// Renderer.cpp

module;

#include <iostream>
#include <memory>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

module Renderer;

Renderer::Renderer(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

void Renderer::Render() const
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (PipelineContainer const & container : m_pipeline_containers)
	{
		GraphicsPipeline const & pipeline = container.m_pipeline;
		pipeline.Activate();
		pipeline.UpdatePerFrameConstants();

		for (std::weak_ptr<RenderObject> render_object : container.m_render_objects)
		{
			std::shared_ptr<RenderObject> obj = render_object.lock();
			if (!obj)
				continue;

			int mesh_id = obj->GetMeshId();
			if (mesh_id == -1)
				continue;

			pipeline.UpdatePerObjectConstants(*obj);

			Mesh const & mesh = m_meshes[mesh_id];
			mesh.Render(obj->GetDrawWireframe());
		}
	}
}

int Renderer::AddPipeline(GraphicsPipeline && pipeline)
{
	if (!pipeline.IsValid())
	{
		std::cout << "Renderer::AddGraphicsPipeline() invalid pipeline";
		return -1;
	}

	m_pipeline_containers.emplace_back(PipelineContainer{
		.m_pipeline{ std::move(pipeline) }
		});

	return static_cast<int>(m_pipeline_containers.size() - 1);
}

int Renderer::AddMesh(Mesh && mesh)
{
	m_meshes.emplace_back(std::move(mesh));
	return static_cast<int>(m_meshes.size() - 1);
}

void Renderer::UpdateMesh(int index, Mesh && mesh)
{
	m_meshes[index] = std::move(mesh);
}

std::shared_ptr<RenderObject> Renderer::CreateRenderObject(std::string const & name, int mesh_id, int pipeline_id, int tex_id /*= -1*/)
{
	if (mesh_id < 0 || mesh_id >= static_cast<int>(m_meshes.size()))
	{
		std::cout << "Renderer::CreateRenderObject() invalid mesh id for object: " << name << std::endl;
		return nullptr;
	}
	if (pipeline_id < 0 || pipeline_id >= static_cast<int>(m_pipeline_containers.size()))
	{
		std::cout << "Renderer::CreateRenderObject() invalid pipeline id for object: " << name << std::endl;
		return nullptr;
	}

	std::shared_ptr<RenderObject> obj = std::make_shared<RenderObject>(name, mesh_id, pipeline_id, tex_id);
	m_pipeline_containers[pipeline_id].m_render_objects.push_back(obj);
	return obj;
}
