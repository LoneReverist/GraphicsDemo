// Mesh.ixx

module;

#include <cstdint>
#include <expected>
#include <vector>

#include <glad/glad.h>

export module Dreamhearth:Mesh;

import :Buffer;
import :RenderContext;
import :GraphicsError;
import :VertexLayout;

namespace Dreamhearth
{
	class VertexArrayObject
	{
	public:
		VertexArrayObject() = default;
		~VertexArrayObject();

		VertexArrayObject(VertexArrayObject && other);
		VertexArrayObject & operator=(VertexArrayObject && other);

		VertexArrayObject(VertexArrayObject const &) = delete;
		VertexArrayObject & operator=(VertexArrayObject const &) = delete;

		void Create();

		unsigned int GetId() const { return m_id; }

	private:
		unsigned int m_id = 0;
	};

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

		template <VertexWithLayout VertexT, typename InstanceT>
			std::expected<void, GraphicsError> Create(
				std::vector<VertexT> const & vertices,
				std::vector<IndexT> const & indices,
				std::vector<InstanceT> const & instances);

		bool IsInitialized() const;

		void Render() const;

	private:
		std::reference_wrapper<RenderContext const> m_render_context;

		Buffer m_vertex_buffer;
		Buffer m_element_buffer;
		Buffer m_instance_buffer;
		VertexArrayObject m_vao;

		GLsizei m_index_count = 0;
		GLsizei m_instance_count = 0;
	};

	Mesh::Mesh(RenderContext const & render_context)
		: m_render_context{ render_context }
	{
	}

	template <VertexWithLayout VertexT>
	std::expected<void, GraphicsError> Mesh::Create(
		std::vector<VertexT> const & vertices,
		std::vector<IndexT> const & indices)
	{
		if (vertices.empty() || indices.empty())
			return std::unexpected{ GraphicsError{ "Mesh::Create: invalid vertices or indicies." } };

		m_vertex_buffer.Create();
		m_element_buffer.Create();
		m_vao.Create();

		glBindVertexArray(m_vao.GetId());

		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer.GetId());
		GLsizeiptr buffer_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexT));
		glBufferData(GL_ARRAY_BUFFER, buffer_size, vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer.GetId());
		buffer_size = static_cast<GLsizeiptr>(indices.size() * sizeof(IndexT));
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, indices.data(), GL_STATIC_DRAW);

		SetAttributes(VertexT::CreateLayout());

		glBindVertexArray(0);

		m_index_count = static_cast<GLsizei>(indices.size());
		m_instance_count = 1;

		return {};
	}

	template <VertexWithLayout VertexT, typename InstanceT>
	std::expected<void, GraphicsError> Mesh::Create(
		std::vector<VertexT> const & vertices,
		std::vector<IndexT> const & indices,
		std::vector<InstanceT> const & instances)
	{
		if (vertices.empty() || indices.empty() || instances.empty())
			return std::unexpected{ GraphicsError{ "Mesh::Create: invalid vertices or indicies." } };

		m_vertex_buffer.Create();
		m_element_buffer.Create();
		m_instance_buffer.Create();
		m_vao.Create();

		glBindVertexArray(m_vao.GetId());

		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer.GetId());
		GLsizeiptr buffer_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexT));
		glBufferData(GL_ARRAY_BUFFER, buffer_size, vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer.GetId());
		buffer_size = static_cast<GLsizeiptr>(indices.size() * sizeof(IndexT));
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, indices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_instance_buffer.GetId());
		buffer_size = static_cast<GLsizeiptr>(instances.size() * sizeof(InstanceT));
		glBufferData(GL_ARRAY_BUFFER, buffer_size, instances.data(), GL_STATIC_DRAW);

		SetAttributes(VertexT::CreateLayout());
		SetAttributes(InstanceT::CreateLayout());

		glBindVertexArray(0);

		m_index_count = static_cast<GLsizei>(indices.size());
		m_instance_count = static_cast<GLsizei>(instances.size());

		return {};
	}
} // namespace Dreamhearth
