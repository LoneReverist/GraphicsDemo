// Vertex.ixx

module;

#include <concepts>
#include <vector>

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

export module Vertex;

export struct PositionVertex {
	glm::vec3 m_pos;
};

export struct NormalVertex {
	glm::vec3 m_pos;
	glm::vec3 m_normal;
};

export struct TextureVertex {
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec2 m_tex_coord;
};

export struct ColorVertex {
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec3 m_color;
};

export template <typename T>
concept IsVertex =
	std::same_as<T, PositionVertex>
	|| std::same_as<T, NormalVertex>
	|| std::same_as<T, TextureVertex>
	|| std::same_as<T, ColorVertex>;

export template <typename T>
concept VertexSupportsNormal = IsVertex<T> && requires(T v) { v.m_normal; };

export template <typename T>
concept VertexSupportsTexCoord = IsVertex<T> && requires(T v) { v.m_tex_coord; };

export template <typename T>
concept VertexSupportsColor = IsVertex<T> && requires(T v) { v.m_color; };

namespace Vertex
{
	export template<IsVertex VertexT>
	void SetAttributes()
	{
		GLsizei stride = sizeof(VertexT);
		size_t offset = 0;
		glVertexAttribPointer(
			0,				// layout (location = 0) in vertex shader
			3,				// 3 floats per vertex position
			GL_FLOAT,		// data type
			GL_FALSE,		// normalize (not relevant)
			stride,			// size of vertex
			reinterpret_cast<void *>(offset));
		glEnableVertexAttribArray(0);
		offset += sizeof(glm::vec3);

		if constexpr (VertexSupportsNormal<VertexT>)
		{
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offset));
			glEnableVertexAttribArray(1);
			offset += sizeof(glm::vec3);
		}

		if constexpr (VertexSupportsTexCoord<VertexT>)
		{
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offset));
			glEnableVertexAttribArray(2);
			offset += sizeof(glm::vec2);
		}

		if constexpr (VertexSupportsColor<VertexT>)
		{
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offset));
			glEnableVertexAttribArray(3);
			//offset += sizeof(glm::vec3);
		}
	}
}
