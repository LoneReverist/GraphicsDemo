// Vertex.ixx

module;

#include <concepts>
#include <cstdint>
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
	export template <IsVertex VertexT>
		VkVertexInputBindingDescription GetBindingDesc()
	{
		return VkVertexInputBindingDescription{
			.binding = 0,
			.stride = sizeof(VertexT),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
	}

	export template <IsVertex VertexT>
		std::vector<VkVertexInputAttributeDescription> GetAttribDescs()
	{
		std::vector<VkVertexInputAttributeDescription> attrib_descs;

		attrib_descs.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<std::uint32_t>(attrib_descs.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(VertexT, m_pos)
			});

		if constexpr (VertexSupportsNormal<VertexT>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<std::uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(VertexT, m_normal),
				});
		}

		if constexpr (VertexSupportsTexCoord<VertexT>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<std::uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(VertexT, m_tex_coord)
				});
		}

		if constexpr (VertexSupportsColor<VertexT>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<std::uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(VertexT, m_color)
				});
		}

		return attrib_descs;
	}
}
