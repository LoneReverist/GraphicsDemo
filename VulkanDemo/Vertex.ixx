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

export template <typename T>
concept VertexConcept = std::same_as<T, PositionVertex> || std::same_as<T, NormalVertex> || std::same_as<T, TextureVertex>;

export template <typename T>
concept VertexSupportsNormal = VertexConcept<T> && requires(T v) { v.m_normal; };

export template <typename T>
concept VertexSupportsTexCoord = VertexConcept<T> && requires(T v) { v.m_tex_coord; };

namespace Vertex
{
	export template <VertexConcept Vertex>
		VkVertexInputBindingDescription GetBindingDesc()
	{
		return VkVertexInputBindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
	}

	export template <VertexConcept Vertex>
		std::vector<VkVertexInputAttributeDescription> GetAttribDescs()
	{
		std::vector<VkVertexInputAttributeDescription> attrib_descs;

		attrib_descs.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(attrib_descs.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, m_pos)
			});

		if constexpr (VertexSupportsNormal<Vertex>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, m_normal),
				});
		}

		if constexpr (VertexSupportsTexCoord<Vertex>)
		{
			attrib_descs.emplace_back(VkVertexInputAttributeDescription{
				.location = static_cast<uint32_t>(attrib_descs.size()),
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, m_tex_coord)
				});
		}

		return attrib_descs;
	}
}