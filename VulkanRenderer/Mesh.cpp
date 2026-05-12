// Mesh.cpp

module;

#include <cstdint>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

module Dreamhearth;

import :Mesh;

namespace Dreamhearth
{
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

		vk::raii::CommandBuffer const & command_buffer = m_render_context.get().GetCurCommandBuffer();

		if (m_instance_buffer.Get() != nullptr && m_instance_buffer.GetMemory() != nullptr)
		{
			vk::DeviceSize offsets[] = { 0, 0 };
			vk::Buffer vertex_buffers[] = { *m_vertex_buffer.Get(), *m_instance_buffer.Get() };
			command_buffer.bindVertexBuffers(0 /*firstBinding*/, vertex_buffers, offsets);
		}
		else
		{
			command_buffer.bindVertexBuffers(0 /*firstBinding*/, *m_vertex_buffer.Get(), vk::DeviceSize{ 0 } /*offsets*/);
		}

		static_assert(std::same_as<IndexT, std::uint16_t>);
		command_buffer.bindIndexBuffer(m_index_buffer.Get(), vk::DeviceSize{ 0 } /*offset*/, vk::IndexType::eUint16);

		command_buffer.drawIndexed(
			m_index_count,
			m_instance_count,
			0 /*firstIndex*/,
			0 /*vertexOffset*/,
			0 /*firstInstance*/);
	}
} // namespace Dreamhearth
