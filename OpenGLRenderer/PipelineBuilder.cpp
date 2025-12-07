// PipelineBuilder.cpp

module;

#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>

module PipelineBuilder;

import GraphicsApi;
import GraphicsError;

std::expected<std::vector<char>, GraphicsError> read_file(std::filesystem::path const & path)
{
	std::ifstream file(path.string(), std::ios::ate | std::ios::binary);
	if (!file.is_open())
		return std::unexpected{ GraphicsError{ "Failed to open file for reading: " + path.string() } };

	size_t file_size = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);
	file.close();

	return buffer;
}

std::expected<unsigned int, GraphicsError> load_shader(
	GLenum shader_type,
	std::filesystem::path const & shader_path)
{
	std::expected<std::vector<char>, GraphicsError> read_result = read_file(shader_path);
	if (!read_result.has_value())
		return std::unexpected{ read_result.error() };

	std::vector<char> & file_data = read_result.value();
	if (file_data.empty())
		return std::unexpected{ GraphicsError{ "Shader file was empty: " + shader_path.string() } };

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
		return std::unexpected{ GraphicsError{ "Failed to compile shader: " + shader_path.string() + "\n" + info_log } };
	}

	return shader_id;
}

PipelineBuilder::~PipelineBuilder()
{
	glDeleteShader(m_vert_shader_id);
	m_vert_shader_id = 0;
	glDeleteShader(m_frag_shader_id);
	m_frag_shader_id = 0;
}

std::expected<void, GraphicsError> PipelineBuilder::LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path)
{
	std::expected<unsigned int, GraphicsError> vert_shader_result = load_shader(GL_VERTEX_SHADER, vs_path);
	if (!vert_shader_result.has_value())
		return std::unexpected{ vert_shader_result.error() };

	std::expected<unsigned int, GraphicsError> frag_shader_result = load_shader(GL_FRAGMENT_SHADER, fs_path);
	if (!frag_shader_result.has_value())
		return std::unexpected{ frag_shader_result.error() };

	m_vert_shader_id = vert_shader_result.value();
	m_frag_shader_id = frag_shader_result.value();

	return {};
}

std::expected<GraphicsPipeline, GraphicsError> PipelineBuilder::CreatePipeline() const
{
	if (m_vert_shader_id == 0)
		return std::unexpected{ GraphicsError{ "Vertex shader not loaded" } };
	if (m_frag_shader_id == 0)
		return std::unexpected{ GraphicsError{ "Fragment shader not loaded" } };
	if (!m_cull_mode.has_value())
		return std::unexpected{ GraphicsError{ "Cull mode not set" } };

	GraphicsPipeline pipeline{
		m_per_frame_constants_callback,
		m_per_object_constants_callback
	};

	std::expected<void, GraphicsError> result = pipeline.Create(
		m_vert_shader_id,
		m_frag_shader_id,
		m_vs_object_uniform_size,
		m_fs_object_uniform_size,
		m_vs_uniform_sizes,
		m_fs_uniform_sizes,
		m_texture,
		m_depth_test_options,
		m_blend_options,
		m_cull_mode.value());
	if (!result.has_value())
		return std::unexpected{ result.error() };

	return pipeline;
}
