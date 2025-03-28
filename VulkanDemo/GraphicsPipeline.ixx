// GraphicsPipeline.ixx

module;

#include <array>
#include <functional>
#include <optional>

#include <vulkan/vulkan.h>

export module GraphicsPipeline;

import GraphicsApi;
import RenderObject;
import Texture;

struct UniformBuffer
{
	VkBuffer m_buffer{ VK_NULL_HANDLE };
	VkDeviceMemory m_memory{ VK_NULL_HANDLE };
	void * m_mapping{ nullptr };
};

struct DescriptorSet
{
	VkDescriptorSet m_descriptor_set{ VK_NULL_HANDLE }; // Automatically cleaned up when m_descriptor_pool is destroyed
	std::vector<UniformBuffer> m_uniform_buffers;
};

export class GraphicsPipeline
{
public:
	using PerFrameConstantsCallback = std::function<void(GraphicsPipeline const & pipeline)>;
	using PerObjectConstantsCallback = std::function<void(GraphicsPipeline const & pipeline, RenderObject const &)>;

	GraphicsPipeline(GraphicsApi const & graphics_api,
		VkShaderModule vert_shader_module,
		VkShaderModule frag_shader_module,
		VkVertexInputBindingDescription const & binding_desc,
		std::vector<VkVertexInputAttributeDescription> const & attrib_descs,
		std::vector<VkPushConstantRange> push_constants_ranges,
		std::vector<VkDeviceSize> vs_uniform_sizes,
		std::vector<VkDeviceSize> fs_uniform_sizes,
		Texture const * texture,
		bool enable_depth_test,
		PerFrameConstantsCallback per_frame_constants_callback,
		PerObjectConstantsCallback per_object_constants_callback);
	~GraphicsPipeline();

	bool IsValid() const { return m_graphics_pipeline != VK_NULL_HANDLE; }

	void Activate() const;
	void UpdatePerFrameConstants() const;
	void UpdatePerObjectConstants(RenderObject const & obj) const;

	template <typename UniformData>
	void SetUniform(uint32_t binding, UniformData const & data) const;

	template <typename VSConstantData, typename FSConstantData = std::nullopt_t>
	void SetPushConstants(VSConstantData const & vs_data, FSConstantData const & fs_data) const;

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
void GraphicsPipeline::SetUniform(uint32_t binding, UniformData const & data) const
{
	UniformBuffer const & buffer = m_descriptor_sets[m_graphics_api.GetCurFrameIndex()].m_uniform_buffers[binding];
	memcpy(buffer.m_mapping, &data, sizeof(data));
}

template <typename VSConstantData, typename FSConstantData /*= std::nullopt_t*/>
void GraphicsPipeline::SetPushConstants(VSConstantData const & vs_data, FSConstantData const & fs_data) const
{
	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();

	vkCmdPushConstants(command_buffer, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT,
		0 /*offset*/, sizeof(VSConstantData), &vs_data);

	if constexpr (!std::same_as<FSConstantData, std::nullopt_t>)
	{
		vkCmdPushConstants(command_buffer, m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT,
			sizeof(VSConstantData) /*offset*/, sizeof(FSConstantData), &fs_data);
	}
}
