// Vertex.ixx

module;

#include <concepts>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

export module Vertex;

import VertexLayout;

export template<typename VertexT>
Vertex::LayoutDesc create_layout()
{
	Vertex::LayoutDesc layout;
	layout.m_stride = sizeof(VertexT);

	std::uint32_t location = 0;

	// All vertex types must have a position attribute.
	Vertex::AttributeType pos_type = Vertex::AttributeType::Float;
	if constexpr (std::same_as<glm::vec3, decltype(VertexT::m_pos)>)
		pos_type = Vertex::AttributeType::Float3;
	else if constexpr (std::same_as<glm::vec2, decltype(VertexT::m_pos)>)
		pos_type = Vertex::AttributeType::Float2;
	else
		static_assert(!std::same_as<VertexT, VertexT>, "Unsupported position format in VertexT");

	layout.m_attributes.push_back(Vertex::AttributeDesc{
		.m_type = pos_type,
		.m_offset = offsetof(VertexT, m_pos),
		.m_location = location++
		});

	if constexpr (requires(VertexT v) { v.m_normal; })
	{
		layout.m_attributes.push_back(Vertex::AttributeDesc{
			.m_type = Vertex::AttributeType::Float3,
			.m_offset = offsetof(VertexT, m_normal),
			.m_location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.m_tex_coord; })
	{
		layout.m_attributes.push_back(Vertex::AttributeDesc{
			.m_type = Vertex::AttributeType::Float2,
			.m_offset = offsetof(VertexT, m_tex_coord),
			.m_location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.m_color; })
	{
		layout.m_attributes.push_back(Vertex::AttributeDesc{
			.m_type = Vertex::AttributeType::Float3,
			.m_offset = offsetof(VertexT, m_color),
			.m_location = location++
			});
	}

	return layout;
}

export struct PositionVertex
{
	glm::vec3 m_pos;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<PositionVertex>(); }
};

export struct NormalVertex
{
	glm::vec3 m_pos;
	glm::vec3 m_normal;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<NormalVertex>(); }
};

export struct TextureVertex
{
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec2 m_tex_coord;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<TextureVertex>(); }
};

export struct ColorVertex
{
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec3 m_color;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<ColorVertex>(); }
};

export struct Texture2dVertex
{
	glm::vec2 m_pos;
	glm::vec2 m_tex_coord;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<Texture2dVertex>(); }
};

export template <typename T>
concept IsVertex =
	std::same_as<T, PositionVertex>
	|| std::same_as<T, NormalVertex>
	|| std::same_as<T, TextureVertex>
	|| std::same_as<T, ColorVertex>
	|| std::same_as<T, Texture2dVertex>;
