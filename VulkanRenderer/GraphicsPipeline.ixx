// GraphicsPipeline.ixx

module;

#include <array>
#include <cstdint>
#include <cstring>
#include <expected>
#include <functional>
#include <optional>

#include <vulkan/vulkan_raii.hpp>

export module GraphicsPipeline;

import Buffer;
import GraphicsApi;
import GraphicsError;
import Texture;

struct UniformBuffer
{
	Buffer buffer;
	void * mapping{ nullptr };
};

struct DescriptorSet
{
	vk::raii::DescriptorSet descriptor_set = nullptr;
	std::vector<UniformBuffer> uniform_buffers;
};

class DescriptorSets
{
public:
	explicit DescriptorSets(GraphicsApi const & graphics_api);

	DescriptorSets(DescriptorSets && other) = default;
	DescriptorSets & operator=(DescriptorSets && other) = default;

	DescriptorSets(DescriptorSets &) = delete;
	DescriptorSets & operator=(DescriptorSets &) = delete;

	void Create(
		std::vector<VkDeviceSize> const & vs_uniform_sizes,
		std::vector<VkDeviceSize> const & fs_uniform_sizes,
		Texture const * texture);

	DescriptorSet const & GetCurrent() const { return m_descriptor_sets[m_graphics_api.get().GetCurFrameIndex()]; }
	vk::raii::DescriptorSetLayout const & GetLayout() const { return m_descriptor_set_layout; }
	vk::raii::DescriptorPool const & GetPool() const { return m_descriptor_pool; }

private:
	std::reference_wrapper<GraphicsApi const> m_graphics_api;

	vk::raii::DescriptorSetLayout m_descriptor_set_layout = nullptr;
	vk::raii::DescriptorPool m_descriptor_pool = nullptr;

	std::array<DescriptorSet, GraphicsApi::m_max_frames_in_flight> m_descriptor_sets;
};

export enum class DepthCompareOp : std::uint8_t
{
	NEVER = static_cast<std::uint8_t>(vk::CompareOp::eNever),
	LESS = static_cast<std::uint8_t>(vk::CompareOp::eLess),
	EQUAL = static_cast<std::uint8_t>(vk::CompareOp::eEqual),
	LESS_OR_EQUAL = static_cast<std::uint8_t>(vk::CompareOp::eLessOrEqual),
	GREATER = static_cast<std::uint8_t>(vk::CompareOp::eGreater),
	NOT_EQUAL = static_cast<std::uint8_t>(vk::CompareOp::eNotEqual),
	GREATER_OR_EQUAL = static_cast<std::uint8_t>(vk::CompareOp::eGreaterOrEqual),
	ALWAYS = static_cast<std::uint8_t>(vk::CompareOp::eAlways)
};

export struct DepthTestOptions
{
	bool enable_depth_test{ true };
	bool enable_depth_write{ true };
	DepthCompareOp depth_compare_op{ DepthCompareOp::LESS };
};

// By default, front facing facets have counter-clockwise vertex windings.
export enum class CullMode : std::uint8_t
{
	NONE = static_cast<std::uint8_t>(vk::CullModeFlagBits::eNone),
	FRONT = static_cast<std::uint8_t>(vk::CullModeFlagBits::eFront),
	BACK = static_cast<std::uint8_t>(vk::CullModeFlagBits::eBack)
};

export enum class BlendFactor : std::uint8_t
{
	ZERO = static_cast<std::uint8_t>(vk::BlendFactor::eZero),
	ONE = static_cast<std::uint8_t>(vk::BlendFactor::eOne),
	SRC_COLOR = static_cast<std::uint8_t>(vk::BlendFactor::eSrcColor),
	ONE_MINUS_SRC_COLOR = static_cast<std::uint8_t>(vk::BlendFactor::eOneMinusSrcColor),
	DST_COLOR = static_cast<std::uint8_t>(vk::BlendFactor::eDstColor),
	ONE_MINUS_DST_COLOR = static_cast<std::uint8_t>(vk::BlendFactor::eOneMinusDstColor),
	SRC_ALPHA = static_cast<std::uint8_t>(vk::BlendFactor::eSrcAlpha),
	ONE_MINUS_SRC_ALPHA = static_cast<std::uint8_t>(vk::BlendFactor::eOneMinusSrcAlpha),
	DST_ALPHA = static_cast<std::uint8_t>(vk::BlendFactor::eDstAlpha),
	ONE_MINUS_DST_ALPHA = static_cast<std::uint8_t>(vk::BlendFactor::eOneMinusDstAlpha),
	CONSTANT_COLOR = static_cast<std::uint8_t>(vk::BlendFactor::eConstantColor),
	ONE_MINUS_CONSTANT_COLOR = static_cast<std::uint8_t>(vk::BlendFactor::eOneMinusConstantColor),
	CONSTANT_ALPHA = static_cast<std::uint8_t>(vk::BlendFactor::eConstantAlpha),
	ONE_MINUS_CONSTANT_ALPHA = static_cast<std::uint8_t>(vk::BlendFactor::eOneMinusConstantAlpha),
	SRC_ALPHA_SATURATE = static_cast<std::uint8_t>(vk::BlendFactor::eSrcAlphaSaturate)
};

export struct BlendOptions
{
	bool enable_blend = false;
	BlendFactor src_factor = BlendFactor::SRC_ALPHA;
	BlendFactor dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA;
};

export class GraphicsPipeline
{
public:
	using PerFrameConstantsCallback = std::function<void(GraphicsPipeline const & pipeline)>;
	using PerObjectConstantsCallback = std::function<void(GraphicsPipeline const & pipeline, void const * object_data)>;

	explicit GraphicsPipeline(
		GraphicsApi const & graphics_api,
		PerFrameConstantsCallback per_frame_constants_callback,
		PerObjectConstantsCallback per_object_constants_callback);
	~GraphicsPipeline() = default;

	GraphicsPipeline(GraphicsPipeline && other) = default;
	GraphicsPipeline & operator=(GraphicsPipeline && other) = default;

	GraphicsPipeline(GraphicsPipeline &) = delete;
	GraphicsPipeline & operator=(GraphicsPipeline &) = delete;

	std::expected<void, GraphicsError> Create(
		std::vector<vk::PipelineShaderStageCreateInfo> shader_stages,
		vk::VertexInputBindingDescription const & binding_desc,
		std::vector<vk::VertexInputAttributeDescription> const & attrib_descs,
		std::vector<vk::PushConstantRange> const & push_constants_ranges,
		std::vector<vk::DeviceSize> const & vs_uniform_sizes,
		std::vector<vk::DeviceSize> const & fs_uniform_sizes,
		Texture const * texture,
		DepthTestOptions const & depth_options,
		BlendOptions const & blend_options,
		CullMode cull_mode);

	void Activate() const;
	void UpdatePerFrameConstants() const;
	void UpdatePerObjectConstants(void const * object_data) const;

	template <typename UniformData>
	void SetUniform(std::uint32_t binding, UniformData const & data) const;

	template <typename ObjectDataVS = std::nullopt_t, typename ObjectDataFS = std::nullopt_t>
	void SetObjectData(ObjectDataVS const & vs_data, ObjectDataFS const & fs_data) const;

private:
	std::reference_wrapper<GraphicsApi const> m_graphics_api;

	vk::raii::PipelineLayout m_pipeline_layout = nullptr;
	vk::raii::Pipeline m_pipeline = nullptr;

	DescriptorSets m_descriptor_sets;

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <typename UniformData>
void GraphicsPipeline::SetUniform(std::uint32_t binding, UniformData const & data) const
{
	UniformBuffer const & buffer = m_descriptor_sets.GetCurrent().uniform_buffers[binding];
	std::memcpy(buffer.mapping, &data, sizeof(data));
}

template <typename ObjectDataVS /*= std::nullopt_t*/, typename ObjectDataFS /*= std::nullopt_t*/>
void GraphicsPipeline::SetObjectData(ObjectDataVS const & vs_data, ObjectDataFS const & fs_data) const
{
	static_assert(!std::same_as<ObjectDataVS, std::nullopt_t> || !std::same_as<ObjectDataFS, std::nullopt_t>,
		"At least one push constant data must be provided");

	vk::raii::CommandBuffer const & command_buffer = m_graphics_api.get().GetCurCommandBuffer();
	std::uint32_t offset = 0;

	if constexpr (!std::same_as<ObjectDataVS, std::nullopt_t>)
	{
		command_buffer.pushConstants<ObjectDataVS>(m_pipeline_layout, vk::ShaderStageFlagBits::eVertex, offset, vs_data);

		offset += static_cast<std::uint32_t>(sizeof(ObjectDataVS));
	}

	if constexpr (!std::same_as<ObjectDataFS, std::nullopt_t>)
	{
		command_buffer.pushConstants<ObjectDataFS>(m_pipeline_layout, vk::ShaderStageFlagBits::eFragment, offset, fs_data);
	}
}
