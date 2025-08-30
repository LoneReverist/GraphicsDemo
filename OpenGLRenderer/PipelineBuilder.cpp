// PipelineBuilder.cpp

module;

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

#include <glad/glad.h>

module PipelineBuilder;

import GraphicsApi;

namespace
{
	std::vector<char> read_file(std::filesystem::path const & path)
	{
		std::ifstream file(path.string(), std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "Failed to open file for reading: " << path << std::endl;
			return std::vector<char>{};
		}

		size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);
		file.close();

		return buffer;
	}

	unsigned int load_shader(
		GLenum shader_type,
		std::filesystem::path const & shader_path)
	{
		std::vector<char> file_data = read_file(shader_path);
		if (file_data.empty())
		{
			std::cout << "File was empty: " << shader_path << std::endl;
			return 0;
		}

		const GLchar * shader_source = file_data.data();
		GLint length = static_cast<GLint>(file_data.size());

		unsigned int shader_id = glCreateShader(shader_type); // returns 0 on error
		glShaderSource(shader_id, 1, &shader_source, &length);
		glCompileShader(shader_id);

		int success = 0;
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char info_log[512];
			glGetShaderInfoLog(shader_id, 512, nullptr, info_log);
			std::cout << "Failed to compile shader: " << shader_path << std::endl << info_log << std::endl;
		}

		return shader_id;
	}
}

PipelineBuilder::~PipelineBuilder()
{
	glDeleteShader(m_vert_shader_id);
	m_vert_shader_id = 0;
	glDeleteShader(m_frag_shader_id);
	m_frag_shader_id = 0;
}

void PipelineBuilder::LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path)
{
	m_vert_shader_id = load_shader(GL_VERTEX_SHADER, vs_path);
	m_frag_shader_id = load_shader(GL_FRAGMENT_SHADER, fs_path);
}

std::optional<GraphicsPipeline> PipelineBuilder::CreatePipeline() const
{
	if (m_vert_shader_id == 0 || m_frag_shader_id == 0)
		return std::nullopt;

	if (m_cull_mode == CullMode::NONE)
		std::cout << "Warning: Building pipeline with cull mode is set to NONE, this may result in suboptimal performance." << std::endl;

	return GraphicsPipeline{
		m_vert_shader_id,
		m_frag_shader_id,
		m_vs_object_uniform_size,
		m_fs_object_uniform_size,
		m_vs_uniform_sizes,
		m_fs_uniform_sizes,
		m_depth_test_options,
		m_blend_options,
		m_cull_mode,
		m_per_frame_constants_callback,
		m_per_object_constants_callback
	};
}
