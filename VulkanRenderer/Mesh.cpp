// Mesh.cpp

module;

#include <cstdint>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

module Mesh;

bool Mesh::IsInitialized() const
{
	return m_vertex_buffer.Get() != nullptr
		&& m_vertex_buffer.GetMemory() != nullptr
		&& m_index_buffer.Get() != nullptr
		&& m_index_buffer.GetMemory() != nullptr
		&& m_index_count > 0;
}

void Mesh::Render() const
{
	if (!IsInitialized())
		return;

	vk::raii::CommandBuffer const & command_buffer = m_graphics_api.get().GetCurCommandBuffer();

	command_buffer.bindVertexBuffers(0 /*firstBinding*/, *m_vertex_buffer.Get(), vk::DeviceSize{ 0 } /*offsets*/);

	static_assert(std::same_as<IndexT, std::uint16_t>);
	command_buffer.bindIndexBuffer(m_index_buffer.Get(), vk::DeviceSize{ 0 } /*offset*/, vk::IndexType::eUint16);

	command_buffer.drawIndexed(
		m_index_count,
		1 /*instanceCount*/,
		0 /*firstIndex*/,
		0 /*vertexOffset*/,
		0 /*firstInstance*/);
}
