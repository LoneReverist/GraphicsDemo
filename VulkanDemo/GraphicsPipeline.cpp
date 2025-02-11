// GraphicsPipeline.cpp

module;

#include <fstream>
#include <iostream>

#include <vulkan/vulkan.h>

#include <glm/gtc/type_ptr.hpp>

module GraphicsPipeline;

//import <fstream>;
//import <iostream>;

import GraphicsApi;

namespace
{
	VkPipelineLayout create_pipeline_layout(VkDevice device)
	{
		VkPipelineLayoutCreateInfo pipeline_layout_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pSetLayouts = nullptr,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr,
		};

		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		VkResult result = vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout);
		if (result != VK_SUCCESS)
			std::cout << "Failed to create pipeline layout" << std::endl;

		return pipeline_layout;
	}

	VkPipeline create_graphics_pipeline(
		VkDevice device,
		VkRenderPass render_pass,
		VkPipelineLayout pipeline_layout,
		std::vector<VkPipelineShaderStageCreateInfo> const & shader_stages)
	{
		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
			.pDynamicStates = dynamic_states.data()
		};

		VkPipelineViewportStateCreateInfo viewport_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1
		};

		//	VkExtent2D sc_extent = graphics_api.GetSwapChainExtent();
		//	VkViewport viewport{
		//		.x = 0.0f,
		//		.y = 0.0f,
		//		.width = static_cast<float>(sc_extent.width),
		//		.height = static_cast<float>(sc_extent.height),
		//		.minDepth = 0.0f,
		//		.maxDepth = 1.0f
		//	};
		//
		//	VkRect2D scissor{
		//		.offset = { 0, 0 },
		//		.extent = sc_extent
		//	};
		//
		//	VkPipelineViewportStateCreateInfo viewport_state{
		//		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		//		.viewportCount = 1,
		//		.pViewports = &viewport,
		//		.scissorCount = 1,
		//		.pScissors = &scissor
		//	};

		VkPipelineVertexInputStateCreateInfo vertex_input_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr
		};

		VkPipelineInputAssemblyStateCreateInfo input_assembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		VkPipelineRasterizationStateCreateInfo rasterizer{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};

		VkPipelineMultisampleStateCreateInfo multisampling{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.0f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		VkPipelineColorBlendAttachmentState color_blend_attachment{ // per framebuffer blending options
			.blendEnable = VK_FALSE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			//.blendEnable = VK_TRUE,
			//.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			//.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			//.colorBlendOp = VK_BLEND_OP_ADD,
			//.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			//.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			//.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		VkPipelineColorBlendStateCreateInfo color_blending{ // global blending options
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &color_blend_attachment,
			.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
		};

		VkGraphicsPipelineCreateInfo pipeline_info{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = static_cast<uint32_t>(shader_stages.size()),
			.pStages = shader_stages.data(),
			.pVertexInputState = &vertex_input_info,
			.pInputAssemblyState = &input_assembly,
			.pViewportState = &viewport_state,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = nullptr,
			.pColorBlendState = &color_blending,
			.pDynamicState = &dynamic_state,
			.layout = pipeline_layout,
			.renderPass = render_pass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1
		};

		VkPipeline graphics_pipeline = VK_NULL_HANDLE;
		VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
		if (result != VK_SUCCESS)
			std::cout << "Failed to create graphics pipeline" << std::endl;

		return graphics_pipeline;
	}
}

GraphicsPipeline::GraphicsPipeline(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

bool GraphicsPipeline::CreatePipeline(
	std::filesystem::path const & vert_shader_path,
	std::filesystem::path const & frag_shader_path)
{
	VkDevice device = m_graphics_api.GetDevice();

	m_pipeline_layout = create_pipeline_layout(device);
	if (m_pipeline_layout == VK_NULL_HANDLE)
		return false;

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

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages{
		vert_shader_stage_info,
		frag_shader_stage_info
	};

	m_graphics_pipeline = create_graphics_pipeline(device, m_graphics_api.GetRenderPass(), m_pipeline_layout, shader_stages);

	vkDestroyShaderModule(device, frag_shader_module, nullptr);
	vkDestroyShaderModule(device, vert_shader_module, nullptr);

	return m_graphics_pipeline != VK_NULL_HANDLE;
}

void GraphicsPipeline::DestroyPipeline()
{
	VkDevice device = m_graphics_api.GetDevice();
	vkDestroyPipeline(device, m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
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
