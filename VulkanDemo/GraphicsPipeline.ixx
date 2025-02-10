// GraphicsPipeline.ixx

module;

#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

export module GraphicsPipeline;

import <filesystem>;

export class GraphicsPipeline
{
public:
	bool LoadPipeline(
		std::filesystem::path const & vert_shader_path,
		std::filesystem::path const & frag_shader_path,
		VkDevice device);
	void DeletePipeline();

	void Activate() const;
	void SetUniform(std::string const & uniform_label, float uniform) const;
	void SetUniform(std::string const & uniform_label, glm::vec3 const & uniform) const;
	void SetUniform(std::string const & uniform_label, glm::mat4 const & uniform) const;

private:
	VkShaderModule load_shader(std::filesystem::path const & shader_path, VkDevice device) const;
	std::vector<char> read_file(std::filesystem::path const & path) const;

private:
	unsigned int m_program_id{ 0 };
};
