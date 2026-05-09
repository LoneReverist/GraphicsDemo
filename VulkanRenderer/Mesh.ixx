// Mesh.ixx

module;

#include <cstdint>
#include <cstring>
#include <expected>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

export module Dreamhearth:Mesh;

import :Buffer;
import :RenderContext;
import :GraphicsError;
import :VertexLayout;

namespace Dreamhearth
{
	export class Mesh
	{
	public:
		using IndexT = std::uint16_t;

		explicit Mesh(RenderContext const & render_context);
		~Mesh() = default;

		Mesh(Mesh && other) = default;
		Mesh & operator=(Mesh && other) = default;

		Mesh(Mesh const &) = delete;
		Mesh & operator=(Mesh const &) = delete;

		template <VertexWithLayout VertexT>
		std::expected<void, GraphicsError> Create(
			std::vector<VertexT> const & vertices,
			std::vector<IndexT> const & indices);

		bool IsInitialized() const;

		void Render() const;

	private:
		std::reference_wrapper<RenderContext const> m_render_context;

		Buffer m_vertex_buffer;
		Buffer m_index_buffer;

		std::uint32_t m_index_count = 0;
	};

	Mesh::Mesh(RenderContext const & render_context)
		: m_render_context{ render_context }
	{
	}

	template <typename T>
	Buffer create_buffer(
		RenderContext const & render_context,
		std::vector<T> objects,
		vk::BufferUsageFlagBits buffer_usage)
	{
		VkDeviceSize buffer_size = sizeof(objects[0]) * objects.size();
		Buffer staging_buffer;
		staging_buffer.Create(
			render_context,
			buffer_size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void * data = staging_buffer.GetMemory().mapMemory(0, buffer_size);
		std::memcpy(data, objects.data(), (size_t)buffer_size);
		staging_buffer.GetMemory().unmapMemory();
		
		Buffer out_buffer;
		out_buffer.Create(
			render_context,
			buffer_size,
			vk::BufferUsageFlagBits::eTransferDst | buffer_usage,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		render_context.CopyBuffer(staging_buffer.Get(), out_buffer.Get(), buffer_size);

		return out_buffer;
	}

	template<VertexWithLayout VertexT>
	std::expected<void, GraphicsError> Mesh::Create(
		std::vector<VertexT> const & vertices,
		std::vector<IndexT> const & indices)
	{
		if (vertices.empty() || indices.empty())
			return std::unexpected{ GraphicsError{ "Mesh::Create: invalid vertices or indicies." } };

		try
		{
			m_vertex_buffer = create_buffer(
				m_render_context.get(),
				vertices,
				vk::BufferUsageFlagBits::eVertexBuffer);

			m_index_buffer = create_buffer(
				m_render_context.get(),
				indices,
				vk::BufferUsageFlagBits::eIndexBuffer);
		}
		catch (vk::SystemError const & err)
		{
			return std::unexpected{ GraphicsError{ "Mesh::Create: failed to create buffers. code: " + std::to_string(err.code().value()) } };
		}

		m_index_count = static_cast<std::uint32_t>(indices.size());

		return {};
	}
} // namespace Dreamhearth
