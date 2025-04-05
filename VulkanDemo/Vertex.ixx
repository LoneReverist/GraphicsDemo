// Vertex.ixx

module;

#include <concepts>
#include <vector>

#include <vulkan/vulkan.h>

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
	export template <IsVertex Vert>
		VkVertexInputBindingDescription GetBindingDesc()
	{
		return VkVertexInputBindingDescription{
			.binding = 0,
			.stride = sizeof(Vert),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
	}

	export template <IsVertex Vert>
		std::vector<VkVertexInputAttributeDescription> GetAttribDescs()
	{
		std::vector<VkVertexInputAttributeDescription> attrib_descs;

		attrib_descs.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<std::uint32_t>(attrib_descs.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vert, m_pos)
			});

		if constexpr (VertexSupportsNormal<Vert>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<std::uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vert, m_normal),
				});
		}

		if constexpr (VertexSupportsTexCoord<Vert>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<std::uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vert, m_tex_coord)
				});
		}

		if constexpr (VertexSupportsColor<Vert>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<std::uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vert, m_color)
				});
		}

		return attrib_descs;
	}
}
