// VertexLayout.ixx

module;

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

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
		std::uint32_t location; // shader location
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

	vk::Format attribute_type_to_vkformat(AttributeType type)
	{
		switch (type)
		{
		case AttributeType::Float:
			return vk::Format::eR32Sfloat;
		case AttributeType::Float2:
			return vk::Format::eR32G32Sfloat;
		case AttributeType::Float3:
			return vk::Format::eR32G32B32Sfloat;
		case AttributeType::Float4:
			return vk::Format::eR32G32B32A32Sfloat;
		default:
			return vk::Format::eUndefined;
		}
	}

	// Get Vulkan binding description from LayoutDesc
	export vk::VertexInputBindingDescription GetBindingDesc(LayoutDesc const & layout)
	{
		return vk::VertexInputBindingDescription{
			.binding = 0,
			.stride = static_cast<std::uint32_t>(layout.stride),
			.inputRate = vk::VertexInputRate::eVertex
		};
	}

	// Get Vulkan attribute descriptions from LayoutDesc
	export std::vector<vk::VertexInputAttributeDescription> GetAttribDescs(LayoutDesc const & layout)
	{
		std::vector<vk::VertexInputAttributeDescription> attribs;
		for (auto const & attr : layout.attributes)
		{
			attribs.emplace_back(vk::VertexInputAttributeDescription{
				.location = attr.location,
				.binding = 0,
				.format = attribute_type_to_vkformat(attr.type),
				.offset = static_cast<std::uint32_t>(attr.offset)
				});
		}
		return attribs;
	}
}
