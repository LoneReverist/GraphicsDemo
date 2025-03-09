// Mesh.ixx

module;

#include <iostream>
#include <variant>
#include <vector>

#include <vulkan/vulkan.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

export module Mesh;

import GraphicsApi;
import Vertex;

template <VertexConcept Vertex>
class MeshImpl
{
public:
	using index_t = uint16_t;

	MeshImpl(
		GraphicsApi const & graphics_api,
		std::vector<Vertex> && vertices,
		std::vector<index_t> && indices);

	void InitBuffers();
	void DeleteBuffers();

	bool IsInitialized() const;

	void Render(bool wireframe) const;

private:
	GraphicsApi const & m_graphics_api;

	std::vector<Vertex> m_vertices;
	std::vector<index_t> m_indices;

	VkBuffer m_vertex_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertex_buffer_memory = VK_NULL_HANDLE;
	VkBuffer m_index_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_index_buffer_memory = VK_NULL_HANDLE;
};

export class Mesh
{
public:
	using index_t = uint16_t;

	template<VertexConcept Vertex>
	Mesh(
		GraphicsApi const & graphics_api,
		std::vector<Vertex> && vertices,
		std::vector<index_t> && indices);

	void InitBuffers();
	void DeleteBuffers();

	void Render(bool wireframe) const;

private:
	using mesh_variant_t = std::variant<
		MeshImpl<PositionVertex>,
		MeshImpl<NormalVertex>,
		MeshImpl<TextureVertex>>;

	mesh_variant_t m_mesh_var;
};

template<VertexConcept Vertex>
Mesh::Mesh(
	GraphicsApi const & graphics_api,
	std::vector<Vertex> && vertices,
	std::vector<index_t> && indices)
	: m_mesh_var(MeshImpl<Vertex>{ graphics_api, std::move(vertices), std::move(indices) })
{}

void Mesh::InitBuffers()
{
	std::visit([](auto & mesh) { mesh.InitBuffers(); }, m_mesh_var);
}

void Mesh::DeleteBuffers()
{
	std::visit([](auto & mesh) { mesh.DeleteBuffers(); }, m_mesh_var);
}

void Mesh::Render(bool wireframe) const
{
	std::visit([wireframe](auto & mesh) { mesh.Render(wireframe); }, m_mesh_var);
}

namespace
{
	template <typename T>
	VkResult init_buffer(
		GraphicsApi const & graphics_api,
		std::vector<T> objects,
		VkBufferUsageFlags buffer_usage,
		VkBuffer & out_buffer,
		VkDeviceMemory & out_buffer_memory
		)
	{
		VkDevice device = graphics_api.GetDevice();

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;

		VkDeviceSize buffer_size = sizeof(objects[0]) * objects.size();
		VkResult result = graphics_api.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_buffer_memory);
		if (result != VK_SUCCESS)
		{
			std::cout << "Failed to create staging buffer" << std::endl;
			return result;
		}

		void * data;
		vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
		memcpy(data, objects.data(), (size_t)buffer_size);
		vkUnmapMemory(device, staging_buffer_memory);

		result = graphics_api.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer_usage,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			out_buffer,
			out_buffer_memory);
		if (result != VK_SUCCESS)
		{
			std::cout << "Failed to create device local buffer" << std::endl;
			return result;
		}

		graphics_api.CopyBuffer(staging_buffer, out_buffer, buffer_size);

		vkDestroyBuffer(device, staging_buffer, nullptr);
		vkFreeMemory(device, staging_buffer_memory, nullptr);

		return VK_SUCCESS;
	}
}

template<VertexConcept Vertex>
MeshImpl<Vertex>::MeshImpl(
	GraphicsApi const & graphics_api,
	std::vector<Vertex> && vertices,
	std::vector<index_t> && indices)
	: m_graphics_api{ graphics_api }
	, m_vertices{ std::move(vertices) }
	, m_indices{ std::move(indices) }
{}

template <VertexConcept Vertex>
void MeshImpl<Vertex>::InitBuffers()
{
	if (m_vertices.empty())
	{
		std::cout << "Mesh::Init() verts empty" << std::endl;
		return;
	}
	if (m_indices.empty())
	{
		std::cout << "Mesh::Init() indices empty" << std::endl;
		return;
	}

	VkResult result = init_buffer(
		m_graphics_api,
		m_vertices,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		m_vertex_buffer,
		m_vertex_buffer_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create vertex buffer" << std::endl;
		return;
	}

	result = init_buffer(
		m_graphics_api,
		m_indices,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		m_index_buffer,
		m_index_buffer_memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create index buffer" << std::endl;
		return;
	}
}

template <VertexConcept Vertex>
void MeshImpl<Vertex>::DeleteBuffers()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroyBuffer(device, m_index_buffer, nullptr);
	m_index_buffer = VK_NULL_HANDLE;
	vkFreeMemory(device, m_index_buffer_memory, nullptr);
	m_index_buffer_memory = VK_NULL_HANDLE;

	vkDestroyBuffer(device, m_vertex_buffer, nullptr);
	m_vertex_buffer = VK_NULL_HANDLE;
	vkFreeMemory(device, m_vertex_buffer_memory, nullptr);
	m_vertex_buffer_memory = VK_NULL_HANDLE;
}

template <VertexConcept Vertex>
bool MeshImpl<Vertex>::IsInitialized() const
{
	return m_vertex_buffer != VK_NULL_HANDLE
		&& m_vertex_buffer_memory != VK_NULL_HANDLE
		&& m_index_buffer != VK_NULL_HANDLE
		&& m_index_buffer_memory != VK_NULL_HANDLE;
}

template <VertexConcept Vertex>
void MeshImpl<Vertex>::Render(bool /*wireframe*/) const
{
	if (!IsInitialized())
		return;

	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();

	VkBuffer vertex_buffers[] = { m_vertex_buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(
		command_buffer,
		0 /*firstBinding*/,
		1 /*bindingCount*/,
		vertex_buffers,
		offsets);

	static_assert(std::same_as<index_t, uint16_t>);
	vkCmdBindIndexBuffer(
		command_buffer,
		m_index_buffer,
		0 /*offset*/,
		VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(
		command_buffer,
		static_cast<uint32_t>(m_indices.size()),
		1 /*instanceCount*/,
		0 /*firstIndex*/,
		0 /*vertexOffset*/,
		0 /*firstInstance*/);
}
