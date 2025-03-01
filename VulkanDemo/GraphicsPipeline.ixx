// GraphicsPipeline.ixx

module;

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

export module GraphicsPipeline;

import <filesystem>;

import GraphicsApi;

export struct PushConstantVSData
{
	alignas(16) glm::mat4 model;
};

export struct PushConstantFSData
{
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 camera_pos_world; // TODO: would make more sense as a descriptor set since it's not per object
};

export struct UniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

export class GraphicsPipeline
{
public:
	GraphicsPipeline(GraphicsApi const & graphics_api);

	bool CreatePipeline(
		std::filesystem::path const & vert_shader_path,
		std::filesystem::path const & frag_shader_path,
		VkVertexInputBindingDescription const & binding_desc,
		std::vector<VkVertexInputAttributeDescription> const & attrib_descs);
	void DestroyPipeline();

	void Activate() const;
	void SetUniform(std::string const & uniform_label, float uniform) const;
	void SetUniform(std::string const & uniform_label, glm::vec3 const & uniform) const;
	void SetUniform(std::string const & uniform_label, glm::mat4 const & uniform) const;

	VkPipelineLayout GetLayout() const { return m_pipeline_layout; }
	void * GetCurMappedUniformBufferObject() const { return m_uniform_buffers_mapped[m_graphics_api.GetCurFrameIndex()]; }
	VkDescriptorSet GetCurDescriptorSet() const { return m_descriptor_sets[m_graphics_api.GetCurFrameIndex()]; }

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
};
