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
	layout.stride = sizeof(VertexT);

	std::uint32_t location = 0;

	// All vertex types must have a position attribute.
	Vertex::AttributeType pos_type = Vertex::AttributeType::Float;
	if constexpr (std::same_as<glm::vec3, decltype(VertexT::pos)>)
		pos_type = Vertex::AttributeType::Float3;
	else if constexpr (std::same_as<glm::vec2, decltype(VertexT::pos)>)
		pos_type = Vertex::AttributeType::Float2;
	else
		static_assert(!std::same_as<VertexT, VertexT>, "Unsupported position format in VertexT");

	layout.attributes.push_back(Vertex::AttributeDesc{
		.type = pos_type,
		.offset = offsetof(VertexT, pos),
		.location = location++
		});

	if constexpr (requires(VertexT v) { v.normal; })
	{
		layout.attributes.push_back(Vertex::AttributeDesc{
			.type = Vertex::AttributeType::Float3,
			.offset = offsetof(VertexT, normal),
			.location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.tex_coord; })
	{
		layout.attributes.push_back(Vertex::AttributeDesc{
			.type = Vertex::AttributeType::Float2,
			.offset = offsetof(VertexT, tex_coord),
			.location = location++
			});
	}

	if constexpr (requires(VertexT v) { v.color; })
	{
		layout.attributes.push_back(Vertex::AttributeDesc{
			.type = Vertex::AttributeType::Float3,
			.offset = offsetof(VertexT, color),
			.location = location++
			});
	}

	return layout;
}

export struct PositionVertex
{
	glm::vec3 pos;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<PositionVertex>(); }
};

export struct NormalVertex
{
	glm::vec3 pos;
	glm::vec3 normal;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<NormalVertex>(); }
};

export struct TextureVertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 tex_coord;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<TextureVertex>(); }
};

export struct ColorVertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<ColorVertex>(); }
};

export struct Texture2dVertex
{
	glm::vec2 pos;
	glm::vec2 tex_coord;

	static Vertex::LayoutDesc CreateLayout() { return create_layout<Texture2dVertex>(); }
};

export template <typename T>
concept IsVertex =
	std::same_as<T, PositionVertex>
	|| std::same_as<T, NormalVertex>
	|| std::same_as<T, TextureVertex>
	|| std::same_as<T, ColorVertex>
	|| std::same_as<T, Texture2dVertex>;
