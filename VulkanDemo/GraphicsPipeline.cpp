// GraphicsPipeline.cpp

module;

#include <fstream>
#include <iostream>

#include <vulkan/vulkan.h>

#include <glm/gtc/type_ptr.hpp>

module GraphicsPipeline;

//import <fstream>;
//import <iostream>;

bool GraphicsPipeline::LoadPipeline(
	std::filesystem::path const & vert_shader_path,
	std::filesystem::path const & frag_shader_path,
	VkDevice device)
{
	VkShaderModule vert_shader_module = load_shader(vert_shader_path, device);
	VkShaderModule frag_shader_module = load_shader(frag_shader_path, device);
	if (vert_shader_module == VK_NULL_HANDLE || frag_shader_module == VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(device, frag_shader_module, nullptr); // TODO: a scope_guard class would be handy
		vkDestroyShaderModule(device, vert_shader_module, nullptr);
		return false;
	}

	VkPipelineShaderStageCreateInfo vert_shader_stage_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vert_shader_module,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo frag_shader_stage_info{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = frag_shader_module,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data(),
	};

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = (float)swapChainExtent.width;
//	viewport.height = (float)swapChainExtent.height;
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	VkRect2D scissor{};
//	scissor.offset = { 0, 0 };
//	scissor.extent = swapChainExtent;
//
//	VkPipelineViewportStateCreateInfo viewportState{};
//	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//	viewportState.viewportCount = 1;
//	viewportState.pViewports = &viewport;
//	viewportState.scissorCount = 1;
//	viewportState.pScissors = &scissor;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;



	vkDestroyShaderModule(device, frag_shader_module, nullptr);
	vkDestroyShaderModule(device, vert_shader_module, nullptr);

	return true;

//	unsigned int vert_shader_id = load_shader(GL_VERTEX_SHADER, vert_shader_path);
//	unsigned int frag_shader_id = load_shader(GL_FRAGMENT_SHADER, frag_shader_path);
//	if (vert_shader_id == 0 || frag_shader_id == 0)
//	{
//		glDeleteShader(vert_shader_id); // TODO: a scope_guard class would be handy
//		glDeleteShader(frag_shader_id);
//		return false;
//	}
//
//	m_program_id = glCreateProgram();
//	glAttachShader(m_program_id, vert_shader_id);
//	glAttachShader(m_program_id, frag_shader_id);
//	glLinkProgram(m_program_id);
//
//	int success = 0;
//	glGetProgramiv(m_program_id, GL_LINK_STATUS, &success);
//	if (!success)
//	{
//		char info_log[512];
//		glGetProgramInfoLog(m_program_id, 512, nullptr, info_log);
//		std::cout << "Failed to link shader program:\n" << info_log << std::endl;
//	}
//
//	glDeleteShader(vert_shader_id);
//	glDeleteShader(frag_shader_id);
//	return success;
}

void GraphicsPipeline::DeletePipeline()
{
//	glDeleteProgram(m_program_id);
}

void GraphicsPipeline::Activate() const
{
//	if (m_program_id == 0)
//	{
//		std::cout << "Activating invalid shader program" << std::endl;
//		return;
//	}
//
//	glUseProgram(m_program_id);
}

void GraphicsPipeline::SetUniform(std::string const & uniform_label, float uniform) const
{
//	GLint uniform_loc = glGetUniformLocation(m_program_id, uniform_label.c_str());
//	if (uniform_loc != -1)
//		glUniform1f(uniform_loc, uniform);
}

void GraphicsPipeline::SetUniform(std::string const & uniform_label, glm::vec3 const & uniform) const
{
//	GLint uniform_loc = glGetUniformLocation(m_program_id, uniform_label.c_str());
//	if (uniform_loc != -1)
//		glUniform3fv(uniform_loc, 1, glm::value_ptr(uniform));
}

void GraphicsPipeline::SetUniform(std::string const & uniform_label, glm::mat4 const & uniform) const
{
//	GLint uniform_loc = glGetUniformLocation(m_program_id, uniform_label.c_str());
//	if (uniform_loc != -1)
//		glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(uniform));
}

VkShaderModule GraphicsPipeline::load_shader(
	std::filesystem::path const & shader_path,
	VkDevice device) const
{
	std::vector<char> file_data = read_file(shader_path);
	if (file_data.empty())
		return VK_NULL_HANDLE;

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

std::vector<char> GraphicsPipeline::read_file(std::filesystem::path const & path) const
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
