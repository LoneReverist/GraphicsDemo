// GraphicsPipeline.ixx

module;

#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

export module GraphicsPipeline;

import <filesystem>;

import GraphicsApi;

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

private:
	VkShaderModule load_shader(std::filesystem::path const & shader_path, VkDevice device) const;
	std::vector<char> read_file(std::filesystem::path const & path) const;

private:
	GraphicsApi const & m_graphics_api;

	VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
	VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;
};
