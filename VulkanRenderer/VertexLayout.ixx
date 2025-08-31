// VertexLayout.ixx

module;

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

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
		AttributeType m_type;
		std::size_t m_offset;
		std::uint32_t m_location; // shader location
	};

	export struct LayoutDesc
	{
		std::size_t m_stride;
		std::vector<AttributeDesc> m_attributes;
	};

	export template<typename VertexT>
	concept VertexWithLayout = requires {
		{ VertexT::CreateLayout() } -> std::same_as<LayoutDesc>;
	};

	VkFormat attribute_type_to_vkformat(AttributeType type)
	{
		switch (type)
		{
		case AttributeType::Float:
			return VK_FORMAT_R32_SFLOAT;
		case AttributeType::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
		case AttributeType::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case AttributeType::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	// Get Vulkan binding description from LayoutDesc
	export VkVertexInputBindingDescription GetBindingDesc(LayoutDesc const & layout)
	{
		return VkVertexInputBindingDescription{
			.binding = 0,
			.stride = static_cast<std::uint32_t>(layout.m_stride),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
	}

	// Get Vulkan attribute descriptions from LayoutDesc
	export std::vector<VkVertexInputAttributeDescription> GetAttribDescs(LayoutDesc const & layout)
	{
		std::vector<VkVertexInputAttributeDescription> attribs;
		for (auto const & attr : layout.m_attributes)
		{
			attribs.emplace_back(VkVertexInputAttributeDescription{
				.location = attr.m_location,
				.binding = 0,
				.format = attribute_type_to_vkformat(attr.m_type),
				.offset = static_cast<std::uint32_t>(attr.m_offset)
				});
		}
		return attribs;
	}
}
