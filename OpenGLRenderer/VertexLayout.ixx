// VertexLayout.ixx

module;

#include <cstdint>
#include <vector>

#include <glad/glad.h>

export module Dreamhearth:VertexLayout;

namespace Dreamhearth
{
	export enum class AttributeType : std::uint8_t
	{
		Float,
		Float2,
		Float3,
		Float4,
	};

	export enum class InputRate : std::uint8_t
	{
		Vertex,
		Instance,
	};

	export struct AttributeDesc
	{
		AttributeType type = AttributeType::Float;
		std::size_t offset = 0;
		uint32_t location = 0; // shader location
	};

	export struct LayoutDesc
	{
		std::uint32_t binding = 0; // for compatibility with Vulkan, not used in OpenGL
		std::size_t stride = 0;
		InputRate input_rate = InputRate::Vertex;

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

			glEnableVertexAttribArray(attr.location);
			glVertexAttribPointer(
				attr.location,
				size,
				type,
				normalize,
				static_cast<GLsizei>(layout.stride),
				reinterpret_cast<const void *>(attr.offset));

			if (layout.input_rate == InputRate::Instance)
				glVertexAttribDivisor(attr.location, 1);
			else
				glVertexAttribDivisor(attr.location, 0);
		}
	}
} // namespace Dreamhearth
