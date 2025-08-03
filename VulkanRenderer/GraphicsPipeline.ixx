// GraphicsPipeline.ixx

module;

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>

#include <vulkan/vulkan.h>

export module GraphicsPipeline;

import GraphicsApi;
import RenderObject;
import Texture;

struct UniformBuffer
{
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	void * m_mapping{ nullptr };
};

struct DescriptorSet
{
	VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE; // Automatically cleaned up when m_descriptor_pool is destroyed
	std::vector<UniformBuffer> m_uniform_buffers;
};

export enum class DepthCompareOp
{
	NEVER = VK_COMPARE_OP_NEVER,
	LESS = VK_COMPARE_OP_LESS,
	EQUAL = VK_COMPARE_OP_EQUAL,
	LESS_OR_EQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
	GREATER = VK_COMPARE_OP_GREATER,
	NOT_EQUAL = VK_COMPARE_OP_NOT_EQUAL,
	GREATER_OR_EQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
	ALWAYS = VK_COMPARE_OP_ALWAYS
};

export struct DepthTestOptions
{
	bool m_enable_depth_test{ true };
	bool m_enable_depth_write{ true };
	DepthCompareOp m_depth_compare_op{ DepthCompareOp::LESS };
};

// By default, front facing facets have counter-clockwise vertex windings.
export enum class CullMode
{
	NONE = VK_CULL_MODE_NONE,
	FRONT = VK_CULL_MODE_FRONT_BIT,
	BACK = VK_CULL_MODE_BACK_BIT
};

export enum class BlendFactor {
	ZERO = VK_BLEND_FACTOR_ZERO,
	ONE = VK_BLEND_FACTOR_ONE,
	SRC_COLOR = VK_BLEND_FACTOR_SRC_COLOR,
	ONE_MINUS_SRC_COLOR = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	DST_COLOR = VK_BLEND_FACTOR_DST_COLOR,
	ONE_MINUS_DST_COLOR = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	SRC_ALPHA = VK_BLEND_FACTOR_SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = VK_BLEND_FACTOR_DST_ALPHA,
	ONE_MINUS_DST_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	CONSTANT_COLOR = VK_BLEND_FACTOR_CONSTANT_COLOR,
	ONE_MINUS_CONSTANT_COLOR = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	CONSTANT_ALPHA = VK_BLEND_FACTOR_CONSTANT_ALPHA,
	ONE_MINUS_CONSTANT_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	SRC_ALPHA_SATURATE = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE
};

export struct BlendOptions {
	bool m_enable_blend = false;
	BlendFactor m_src_factor = BlendFactor::SRC_ALPHA;
	BlendFactor m_dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA;
};

export class GraphicsPipeline
{
public:
	using PerFrameConstantsCallback = std::function<void(GraphicsPipeline const & pipeline)>;
	using PerObjectConstantsCallback = std::function<void(GraphicsPipeline const & pipeline, RenderObject const &)>;

	GraphicsPipeline(
		GraphicsApi const & graphics_api,
		VkShaderModule vert_shader_module,
		VkShaderModule frag_shader_module,
		VkVertexInputBindingDescription const & binding_desc,
		std::vector<VkVertexInputAttributeDescription> const & attrib_descs,
		std::vector<VkPushConstantRange> push_constants_ranges,
		std::vector<VkDeviceSize> vs_uniform_sizes,
		std::vector<VkDeviceSize> fs_uniform_sizes,
		Texture const * texture,
		DepthTestOptions const & depth_options,
		BlendOptions const & blend_options,
		CullMode cull_mode,
		PerFrameConstantsCallback per_frame_constants_callback,
		PerObjectConstantsCallback per_object_constants_callback);
	~GraphicsPipeline();

	GraphicsPipeline(GraphicsPipeline && other);
	GraphicsPipeline & operator=(GraphicsPipeline && other);

	GraphicsPipeline(GraphicsPipeline &) = delete;
	GraphicsPipeline & operator=(GraphicsPipeline &) = delete;

	bool IsValid() const { return m_graphics_pipeline != VK_NULL_HANDLE; }

	void Activate() const;
	void UpdatePerFrameConstants() const;
	void UpdatePerObjectConstants(RenderObject const & obj) const;

	template <typename UniformData>
	void SetUniform(std::uint32_t binding, UniformData const & data) const;

	template <typename VSConstantData = std::nullopt_t, typename FSConstantData = std::nullopt_t>
	void SetPushConstants(VSConstantData const & vs_data, FSConstantData const & fs_data) const;

private:
	void destroy_pipeline();

private:
	GraphicsApi const & m_graphics_api;

	VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
	VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;

	VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
	VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;

	std::array<DescriptorSet, GraphicsApi::m_max_frames_in_flight> m_descriptor_sets;

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <typename UniformData>
void GraphicsPipeline::SetUniform(std::uint32_t binding, UniformData const & data) const
{
	UniformBuffer const & buffer = m_descriptor_sets[m_graphics_api.GetCurFrameIndex()].m_uniform_buffers[binding];
	std::memcpy(buffer.m_mapping, &data, sizeof(data));
}

template <typename VSConstantData /*= std::nullopt_t*/, typename FSConstantData /*= std::nullopt_t*/>
void GraphicsPipeline::SetPushConstants(VSConstantData const & vs_data, FSConstantData const & fs_data) const
{
	static_assert(!std::same_as<VSConstantData, std::nullopt_t> || !std::same_as<FSConstantData, std::nullopt_t>,
		"At least one push constant data must be provided");

	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();
	std::uint32_t offset = 0;

	if constexpr (!std::same_as<VSConstantData, std::nullopt_t>)
	{
		vkCmdPushConstants(command_buffer, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT,
			offset, sizeof(VSConstantData), &vs_data);

		offset += static_cast<std::uint32_t>(sizeof(VSConstantData));
	}

	if constexpr (!std::same_as<FSConstantData, std::nullopt_t>)
	{
		vkCmdPushConstants(command_buffer, m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT,
			offset, sizeof(FSConstantData), &fs_data);
	}
}
