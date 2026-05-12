// VertexLayout.ixx

module;

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

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
		Vertex = static_cast<std::uint8_t>(vk::VertexInputRate::eVertex),
		Instance = static_cast<std::uint8_t>(vk::VertexInputRate::eInstance)
	};

	export struct AttributeDesc
	{
		AttributeType type = AttributeType::Float;
		std::size_t offset = 0;
		std::uint32_t location = 0; // shader location
	};

	export struct LayoutDesc
	{
		std::uint32_t binding = 0;
		std::size_t stride = 0;
		InputRate input_rate = InputRate::Vertex;

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
			.binding = layout.binding,
			.stride = static_cast<std::uint32_t>(layout.stride),
			.inputRate = static_cast<vk::VertexInputRate>(layout.input_rate)
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
				.binding = layout.binding,
				.format = attribute_type_to_vkformat(attr.type),
				.offset = static_cast<std::uint32_t>(attr.offset)
				});
		}
		return attribs;
	}
} // namespace Dreamhearth
