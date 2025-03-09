// PipelineBuilder.cpp

module;

#include <fstream>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

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

	VkShaderModule load_shader(
		std::filesystem::path const & shader_path,
		VkDevice device)
	{
		std::vector<char> file_data = read_file(shader_path);
		if (file_data.empty())
		{
			std::cout << "File was empty: " << shader_path << std::endl;
			return VK_NULL_HANDLE;
		}
		if (file_data.size() % sizeof(uint32_t) != 0) // SPIR-V files should be 32-bit
		{
			std::cout << "File is not 32-bit: " << shader_path << std::endl;
			return VK_NULL_HANDLE;
		}

		VkShaderModuleCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = file_data.size(),
			.pCode = reinterpret_cast<const uint32_t *>(file_data.data())
		};

		VkShaderModule shader_module = VK_NULL_HANDLE;
		VkResult result = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
		if (result != VK_SUCCESS)
			std::cout << "Failed to create shader module: " << shader_path << std::endl;

		return shader_module;
	}
}

PipelineBuilder::PipelineBuilder(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

PipelineBuilder::~PipelineBuilder()
{
	VkDevice device = m_graphics_api.GetDevice();

	vkDestroyShaderModule(device, m_frag_shader_module, nullptr);
	vkDestroyShaderModule(device, m_vert_shader_module, nullptr);
}

void PipelineBuilder::LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path)
{
	VkDevice device = m_graphics_api.GetDevice();

	m_vert_shader_module = load_shader(vs_path, device);
	m_frag_shader_module = load_shader(fs_path, device);
}

std::unique_ptr<GraphicsPipeline> PipelineBuilder::CreatePipeline() const
{
	if (m_vert_shader_module == VK_NULL_HANDLE
		|| m_frag_shader_module == VK_NULL_HANDLE
		|| m_vert_attrib_descs.empty())
	{
		return nullptr;
	}

	return std::make_unique<GraphicsPipeline>(
		m_graphics_api,
		m_vert_shader_module,
		m_frag_shader_module,
		m_vert_binding_desc,
		m_vert_attrib_descs,
		m_push_constants_ranges,
		m_uniform_size,
		m_per_frame_constants_callback,
		m_per_object_constants_callback);
}
