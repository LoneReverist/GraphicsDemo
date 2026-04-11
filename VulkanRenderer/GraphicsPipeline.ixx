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
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE; // Automatically cleaned up when m_descriptor_pool is destroyed
	std::vector<UniformBuffer> uniform_buffers;
};

class DescriptorSets
{
public:
	explicit DescriptorSets(GraphicsApi const & graphics_api);
	~DescriptorSets();

	DescriptorSets(DescriptorSets && other);
	DescriptorSets & operator=(DescriptorSets && other);

	DescriptorSets(DescriptorSets &) = delete;
	DescriptorSets & operator=(DescriptorSets &) = delete;

	std::expected<void, GraphicsError> Create(
		std::vector<VkDeviceSize> const & vs_uniform_sizes,
		std::vector<VkDeviceSize> const & fs_uniform_sizes,
		Texture const * texture);
	void Destroy();

	DescriptorSet const & GetCurrent() const { return m_descriptor_sets[m_graphics_api.GetCurFrameIndex()]; }
	VkDescriptorSetLayout GetLayout() const { return m_descriptor_set_layout; }
	VkDescriptorPool GetPool() const { return m_descriptor_pool; }

private:
	GraphicsApi const & m_graphics_api;

	VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
	VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;

	std::array<DescriptorSet, GraphicsApi::m_max_frames_in_flight> m_descriptor_sets;
};

export enum class DepthCompareOp
{
	NEVER = vk::CompareOp::eNever,
	LESS = vk::CompareOp::eLess,
	EQUAL = vk::CompareOp::eEqual,
	LESS_OR_EQUAL = vk::CompareOp::eLessOrEqual,
	GREATER = vk::CompareOp::eGreater,
	NOT_EQUAL = vk::CompareOp::eNotEqual,
	GREATER_OR_EQUAL = vk::CompareOp::eGreaterOrEqual,
	ALWAYS = vk::CompareOp::eAlways
};

export struct DepthTestOptions
{
	bool enable_depth_test{ true };
	bool enable_depth_write{ true };
	DepthCompareOp depth_compare_op{ DepthCompareOp::LESS };
};

// By default, front facing facets have counter-clockwise vertex windings.
export enum class CullMode
{
	NONE = vk::CullModeFlagBits::eNone,
	FRONT = vk::CullModeFlagBits::eFront,
	BACK = vk::CullModeFlagBits::eBack
};

export enum class BlendFactor
{
	ZERO = vk::BlendFactor::eZero,
	ONE = vk::BlendFactor::eOne,
	SRC_COLOR = vk::BlendFactor::eSrcColor,
	ONE_MINUS_SRC_COLOR = vk::BlendFactor::eOneMinusSrcColor,
	DST_COLOR = vk::BlendFactor::eDstColor,
	ONE_MINUS_DST_COLOR = vk::BlendFactor::eOneMinusDstColor,
	SRC_ALPHA = vk::BlendFactor::eSrcAlpha,
	ONE_MINUS_SRC_ALPHA = vk::BlendFactor::eOneMinusSrcAlpha,
	DST_ALPHA = vk::BlendFactor::eDstAlpha,
	ONE_MINUS_DST_ALPHA = vk::BlendFactor::eOneMinusDstAlpha,
	CONSTANT_COLOR = vk::BlendFactor::eConstantColor,
	ONE_MINUS_CONSTANT_COLOR = vk::BlendFactor::eOneMinusConstantColor,
	CONSTANT_ALPHA = vk::BlendFactor::eConstantAlpha,
	ONE_MINUS_CONSTANT_ALPHA = vk::BlendFactor::eOneMinusConstantAlpha,
	SRC_ALPHA_SATURATE = vk::BlendFactor::eSrcAlphaSaturate
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
		vk::ShaderModule vert_shader_module,
		vk::ShaderModule frag_shader_module,
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
