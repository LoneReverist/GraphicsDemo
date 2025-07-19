// Vertex.ixx

module;

#include <concepts>
#include <vector>

#include <glad/glad.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

export module Vertex;

export struct PositionVertex
{
	glm::vec3 m_pos;
};

export struct NormalVertex
{
	glm::vec3 m_pos;
	glm::vec3 m_normal;
};

export struct TextureVertex
{
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec2 m_tex_coord;
};

export struct ColorVertex
{
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec3 m_color;
};

export struct Texture2dVertex
{
	glm::vec2 m_pos;
	glm::vec2 m_tex_coord;
};

export template <typename T>
concept IsVertex =
	std::same_as<T, PositionVertex>
	|| std::same_as<T, NormalVertex>
	|| std::same_as<T, TextureVertex>
	|| std::same_as<T, ColorVertex>
	|| std::same_as<T, Texture2dVertex>;

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
		int location = 0;

		// All vertex types must have a position attribute.
		int pos_num_floats = 0;
		if constexpr (std::same_as<glm::vec3, decltype(VertexT::m_pos)>)
			pos_num_floats = 3;
		else if constexpr (std::same_as<glm::vec2, decltype(VertexT::m_pos)>)
			pos_num_floats = 2;
		else
			static_assert(!std::same_as<VertexT, VertexT>, "Unsupported position format in VertexT");

		glVertexAttribPointer(
			location,		// layout (location = 0) in vertex shader
			pos_num_floats,	// 2/3 floats per vertex position
			GL_FLOAT,		// data type
			GL_FALSE,		// normalize (not relevant)
			stride,			// size of vertex
			reinterpret_cast<void *>(offsetof(VertexT, m_pos)));
		glEnableVertexAttribArray(location++);

		if constexpr (VertexSupportsNormal<VertexT>)
		{
			glVertexAttribPointer(
				location,
				3,
				GL_FLOAT,
				GL_FALSE,
				stride,
				reinterpret_cast<void *>(offsetof(VertexT, m_normal)));
			glEnableVertexAttribArray(location++);
		}

		if constexpr (VertexSupportsTexCoord<VertexT>)
		{
			glVertexAttribPointer(
				location,
				2,
				GL_FLOAT,
				GL_FALSE,
				stride,
				reinterpret_cast<void *>(offsetof(VertexT, m_tex_coord)));
			glEnableVertexAttribArray(location++);
		}

		if constexpr (VertexSupportsColor<VertexT>)
		{
			glVertexAttribPointer(
				location,
				3,
				GL_FLOAT,
				GL_FALSE,
				stride,
				reinterpret_cast<void *>(offsetof(VertexT, m_color)));
			glEnableVertexAttribArray(location++);
		}
	}
}
