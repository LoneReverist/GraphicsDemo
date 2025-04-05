// Renderer.cpp

module;

#include <iostream>

#include <vulkan/vulkan.h>

module Renderer;

import ObjLoader;
import Vertex;

Renderer::Renderer(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

void Renderer::Render() const
{
	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();
	VkExtent2D sc_extent = m_graphics_api.GetSwapChainExtent();

	VkCommandBufferBeginInfo begin_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};

	VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to begin recording command buffer!");

	std::array<VkClearValue, 2> clear_values = {
		VkClearValue{ .color{ m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f } },
		VkClearValue{ .depthStencil{ 1.0f, 0 } }
	};

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
		.clearValueCount = static_cast<uint32_t>(clear_values.size()),
		.pClearValues = clear_values.data()
	};

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	for (PipelineContainer const & container : m_pipeline_containers)
	{
		GraphicsPipeline const & pipeline = *container.m_pipeline;
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

	vkCmdEndRenderPass(command_buffer);

	result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer!");
}

int Renderer::AddGraphicsPipeline(std::unique_ptr<GraphicsPipeline> pipeline)
{
	if (!pipeline || !pipeline->IsValid())
	{
		std::cout << "Renderer::AddGraphicsPipeline() invalid pipeline";
		return -1;
	}

	m_pipeline_containers.emplace_back(PipelineContainer{
		.m_pipeline{ std::move(pipeline) }
		});

	return static_cast<int>(m_pipeline_containers.size() - 1);
}

int Renderer::LoadMesh(std::filesystem::path const & mesh_path)
{
	std::vector<NormalVertex> vertices;
	std::vector<Mesh::index_t> indices;
	if (!ObjLoader::LoadObjFile(mesh_path, vertices, indices))
	{
		std::cout << "Renderer::LoadMesh() error loading file:" << mesh_path << std::endl;
		return -1;
	}

	m_meshes.emplace_back(m_graphics_api, vertices, indices);
	return static_cast<int>(m_meshes.size() - 1);
}

int Renderer::AddMesh(Mesh && mesh)
{
	m_meshes.emplace_back(std::move(mesh));
	return static_cast<int>(m_meshes.size() - 1);
}

std::shared_ptr<RenderObject> Renderer::CreateRenderObject(int mesh_id, int pipeline_id)
{
	// Right now it's up to the developer to ensure the Vertex type of the mesh is the same as
	// the Vertex type for the pipeline, but at some point a static_assert would be good
	std::shared_ptr<RenderObject> obj = std::make_shared<RenderObject>(mesh_id, pipeline_id);
	m_pipeline_containers[pipeline_id].m_render_objects.push_back(obj);
	return obj;
}
