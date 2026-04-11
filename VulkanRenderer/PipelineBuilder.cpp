// PipelineBuilder.cpp

module;

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

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

std::expected<vk::raii::ShaderModule, GraphicsError> load_shader(
	std::filesystem::path const & shader_path,
	vk::raii::Device const & device)
{
	std::expected<std::vector<char>, GraphicsError> read_result = read_file(shader_path);
	if (!read_result.has_value())
		return std::unexpected{ read_result.error() };

	std::vector<char> & file_data = read_result.value();
	if (file_data.empty())
		return std::unexpected{ GraphicsError{ "Shader file was empty: " + shader_path.string() } };
	if (file_data.size() % sizeof(std::uint32_t) != 0) // SPIR-V files should be 32-bit
		return std::unexpected{ GraphicsError{ "Shader file size is not a multiple of 4 bytes: " + shader_path.string() } };

	vk::ShaderModuleCreateInfo create_info{
		.codeSize = file_data.size(),
		.pCode = reinterpret_cast<const std::uint32_t *>(file_data.data())
	};

	return vk::raii::ShaderModule(device, create_info);
}

PipelineBuilder::PipelineBuilder(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

std::expected<void, GraphicsError> PipelineBuilder::LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path)
{
	// In Vulkan, the shaders are precompiled to SPIR-V
	std::filesystem::path vert_spirv_path = vs_path;
	vert_spirv_path.replace_extension(".vert.spv");
	std::expected<vk::raii::ShaderModule, GraphicsError> vert_shader_result = load_shader(vert_spirv_path, m_graphics_api.GetDevice());
	if (!vert_shader_result.has_value())
		return std::unexpected{ vert_shader_result.error() };

	std::filesystem::path frag_spirv_path = fs_path;
	frag_spirv_path.replace_extension(".frag.spv");
	std::expected<vk::raii::ShaderModule, GraphicsError> frag_shader_result = load_shader(frag_spirv_path, m_graphics_api.GetDevice());
	if (!frag_shader_result.has_value())
		return std::unexpected{ frag_shader_result.error() };

	m_vert_shader_module = std::move(vert_shader_result.value());
	m_frag_shader_module = std::move(frag_shader_result.value());

	return {};
}

std::expected<GraphicsPipeline, GraphicsError> PipelineBuilder::CreatePipeline() const
{
	if (m_vert_shader_module == VK_NULL_HANDLE)
		return std::unexpected{ GraphicsError{ "Vertex shader not loaded" } };
	if (m_frag_shader_module == VK_NULL_HANDLE)
		return std::unexpected{ GraphicsError{ "Fragment shader not loaded" } };
	if (m_vert_attrib_descs.empty())
		return std::unexpected{ GraphicsError{ "Vertex input not set" } };
	if (!m_cull_mode.has_value())
		return std::unexpected{ GraphicsError{ "Cull mode not set" } };

	GraphicsPipeline pipeline{
		m_graphics_api,
		m_per_frame_constants_callback,
		m_per_object_constants_callback
	};

	std::expected<void, GraphicsError> result = pipeline.Create(
		m_vert_shader_module,
		m_frag_shader_module,
		m_vert_binding_desc,
		m_vert_attrib_descs,
		m_push_constants_ranges,
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
