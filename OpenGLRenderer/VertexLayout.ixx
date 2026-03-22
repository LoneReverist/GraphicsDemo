// VertexLayout.ixx

module;

#include <cstdint>
#include <vector>

#include <glad/glad.h>

export module VertexLayout;

namespace Vertex
{
	export enum class AttributeType
	{
		Float,
		Float2,
		Float3,
		Float4,
	};

	export struct AttributeDesc
	{
		AttributeType type;
		std::size_t offset;
		uint32_t location; // shader location
	};

	export struct LayoutDesc
	{
		std::size_t stride;
		std::vector<AttributeDesc> attributes;
	};

	export template<typename VertexT>
	concept VertexWithLayout = requires {
		{ VertexT::CreateLayout() } -> std::same_as<LayoutDesc>;
	};

	// Set OpenGL vertex attributes from a VertexLayoutDesc
	export void SetAttributes(const LayoutDesc & layout)
	{
		for (const auto & attr : layout.attributes)
		{
			GLint size = 0;
			GLenum type = GL_FLOAT;
			GLboolean normalize = GL_FALSE;
			switch (attr.type)
			{
			case AttributeType::Float:
				size = 1;
				break;
			case AttributeType::Float2:
				size = 2;
				break;
			case AttributeType::Float3:
				size = 3;
				break;
			case AttributeType::Float4:
				size = 4;
				break;
			}

			glVertexAttribPointer(
				attr.location,
				size,
				type,
				normalize,
				static_cast<GLsizei>(layout.stride),
				reinterpret_cast<const void *>(attr.offset));
			glEnableVertexAttribArray(attr.location);
		}
	}
}
