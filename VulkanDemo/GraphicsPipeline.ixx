// GraphicsPipeline.ixx

module;

#include <array>
#include <functional>

#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

export module GraphicsPipeline;

import <filesystem>;

import GraphicsApi;
import RenderObject;

export struct PushConstantVSData
{
	alignas(16) glm::mat4 model;
};

export struct PushConstantFSData
{
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 camera_pos_world; // TODO: would make more sense as a descriptor set since it's not per object
};

export struct ViewProjUniform
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

export class GraphicsPipeline
{
public:
	using PerFrameConstantsCallback = std::function<void(GraphicsPipeline const & pipeline)>;
	using PerObjectConstantsCallback = std::function<void(GraphicsPipeline const & pipeline, RenderObject const &)>;

	GraphicsPipeline(GraphicsApi const & graphics_api,
		std::filesystem::path const & vert_shader_path,
		std::filesystem::path const & frag_shader_path,
		VkVertexInputBindingDescription const & binding_desc,
		std::vector<VkVertexInputAttributeDescription> const & attrib_descs);
	~GraphicsPipeline();

	bool IsValid() const { return m_graphics_pipeline != VK_NULL_HANDLE; }

	void SetPerFrameConstantsCallback(PerFrameConstantsCallback callback) { m_per_frame_constants_callback = callback; }
	void SetPerObjectConstantsCallback(PerObjectConstantsCallback callback) { m_per_object_constants_callback = callback; }

	void Activate() const;
	void UpdatePerFrameConstants() const;
	void UpdatePerObjectConstants(RenderObject const & obj) const;

	template <typename UniformData>
	void SetUniform(uint32_t index, UniformData const & data) const;

	template <typename VSConstantData, typename FSContantData>
	void SetPushConstants(VSConstantData const & vs_data, FSContantData const & fs_data) const;

private:
	VkShaderModule load_shader(std::filesystem::path const & shader_path, VkDevice device) const;
	std::vector<char> read_file(std::filesystem::path const & path) const;

	void create_uniform_buffers();

private:
	GraphicsApi const & m_graphics_api;

	VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
	VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;

	VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
	VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;
	std::array<VkDescriptorSet, GraphicsApi::m_max_frames_in_flight> m_descriptor_sets; // Automatically cleaned up when m_descriptor_pool is destroyed

	std::array<VkBuffer, GraphicsApi::m_max_frames_in_flight> m_uniform_buffers;
	std::array<VkDeviceMemory, GraphicsApi::m_max_frames_in_flight> m_uniform_buffers_memory;
	std::array<void *, GraphicsApi::m_max_frames_in_flight> m_uniform_buffers_mapped;

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <typename UniformData>
void GraphicsPipeline::SetUniform(uint32_t /*index*/, UniformData const & data) const
{
	void * mapped_buffer = m_uniform_buffers_mapped[m_graphics_api.GetCurFrameIndex()];
	memcpy(mapped_buffer, &data, sizeof(data));
}

template <typename VSConstantData, typename FSContantData>
void GraphicsPipeline::SetPushConstants(VSConstantData const & vs_data, FSContantData const & fs_data) const
{
	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();

	vkCmdPushConstants(command_buffer, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT,
		0 /*offset*/, sizeof(VSConstantData), &vs_data);

	vkCmdPushConstants(command_buffer, m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VSConstantData) /*offset*/, sizeof(FSContantData), &fs_data);
}
