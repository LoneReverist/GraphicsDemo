// GraphicsPipeline.cpp

module;

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

module GraphicsPipeline;

import GraphicsApi;

namespace
{
	VkPipelineLayout create_pipeline_layout(
		VkDevice device,
		VkDescriptorSetLayout descriptor_set_layout,
		std::vector<VkPushConstantRange> push_constants_ranges)
	{
		VkPipelineLayoutCreateInfo pipeline_layout_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptor_set_layout,
			.pushConstantRangeCount = static_cast<std::uint32_t>(push_constants_ranges.size()),
			.pPushConstantRanges = push_constants_ranges.data(),
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
		std::vector<VkPipelineShaderStageCreateInfo> const & shader_stages,
		VkVertexInputBindingDescription const & binding_desc,
		std::vector<VkVertexInputAttributeDescription> const & attrib_descs,
		DepthTestOptions const & depth_options,
		BlendOptions const & blend_options,
		CullMode cull_mode)
	{
		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
			.pDynamicStates = dynamic_states.data()
		};

		VkPipelineViewportStateCreateInfo viewport_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1
		};

		VkPipelineVertexInputStateCreateInfo vertex_input_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &binding_desc,
			.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attrib_descs.size()),
			.pVertexAttributeDescriptions = attrib_descs.data()
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
			.cullMode = static_cast<VkCullModeFlags>(cull_mode),
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

		VkPipelineColorBlendAttachmentState color_blend_attachment{
			.blendEnable = blend_options.m_enable_blend ? VK_TRUE : VK_FALSE,
			.srcColorBlendFactor = static_cast<VkBlendFactor>(blend_options.m_src_factor),
			.dstColorBlendFactor = static_cast<VkBlendFactor>(blend_options.m_dst_factor),
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = static_cast<VkBlendFactor>(blend_options.m_src_factor),
			.dstAlphaBlendFactor = static_cast<VkBlendFactor>(blend_options.m_dst_factor),
			.alphaBlendOp = VK_BLEND_OP_ADD,
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

		VkPipelineDepthStencilStateCreateInfo depth_stencil{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = depth_options.m_enable_depth_test ? VK_TRUE : VK_FALSE,
			.depthWriteEnable = depth_options.m_enable_depth_write ? VK_TRUE : VK_FALSE,
			.depthCompareOp = static_cast<VkCompareOp>(depth_options.m_depth_compare_op),
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE
		};

		VkGraphicsPipelineCreateInfo pipeline_info{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = static_cast<std::uint32_t>(shader_stages.size()),
			.pStages = shader_stages.data(),
			.pVertexInputState = &vertex_input_info,
			.pInputAssemblyState = &input_assembly,
			.pViewportState = &viewport_state,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depth_stencil,
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

	VkDescriptorSetLayout create_descriptor_set_layout(
		VkDevice device,
		std::uint32_t vs_descriptor_set_count,
		std::uint32_t fs_descriptor_set_count,
		bool has_texture)
	{
		std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
		layout_bindings.reserve(vs_descriptor_set_count + fs_descriptor_set_count + (has_texture ? 1 : 0));

		for (std::uint32_t i = 0; i < vs_descriptor_set_count; ++i)
		{
			layout_bindings.emplace_back(
				VkDescriptorSetLayoutBinding{
					.binding = static_cast<std::uint32_t>(layout_bindings.size()),
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.pImmutableSamplers = nullptr
				});
		}
		for (std::uint32_t i = 0; i < fs_descriptor_set_count; ++i)
		{
			layout_bindings.emplace_back(
				VkDescriptorSetLayoutBinding{
					.binding = static_cast<std::uint32_t>(layout_bindings.size()),
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
					.pImmutableSamplers = nullptr
				});
		}

		if (has_texture)
		{
			layout_bindings.emplace_back(
				VkDescriptorSetLayoutBinding{
					.binding = static_cast<std::uint32_t>(layout_bindings.size()),
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
					.pImmutableSamplers = nullptr
				});
		}

		VkDescriptorSetLayoutCreateInfo layout_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<std::uint32_t>(layout_bindings.size()),
			.pBindings = layout_bindings.data()
		};

		VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
		VkResult result = vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout);
		if (result != VK_SUCCESS)
			std::cout << "Failed to create ubo descriptor set layout";

		return descriptor_set_layout;
	}

	VkDescriptorPool create_descriptor_pool(VkDevice device,
		std::uint32_t uniform_count, std::uint32_t descriptor_set_count, bool has_texture)
	{
		std::vector<VkDescriptorPoolSize> pool_sizes;
		
		if (uniform_count > 0)
		{
			pool_sizes.emplace_back(
				VkDescriptorPoolSize{
					.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = uniform_count * descriptor_set_count
				});
		}

		if (has_texture)
		{
			pool_sizes.emplace_back(
				VkDescriptorPoolSize{
					.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = descriptor_set_count
				});
		}

		VkDescriptorPoolCreateInfo pool_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = descriptor_set_count,
			.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
			.pPoolSizes = pool_sizes.data()
		};

		VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
		VkResult result = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);
		if (result != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor pool!");

		return descriptor_pool;
	}

	template <std::uint32_t count>
	std::array<VkDescriptorSet, count> create_descriptor_sets(
		VkDevice device,
		VkDescriptorSetLayout layout,
		VkDescriptorPool pool,
		std::array<std::vector<VkBuffer>, count> uniform_buffers,
		std::vector<VkDeviceSize> uniform_sizes,
		Texture const * texture)
	{
		std::array<VkDescriptorSetLayout, count> layouts;
		layouts.fill(layout);

		VkDescriptorSetAllocateInfo alloc_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = pool,
			.descriptorSetCount = static_cast<std::uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data()
		};

		std::array<VkDescriptorSet, count> descriptor_sets;
		VkResult result = vkAllocateDescriptorSets(device, &alloc_info, descriptor_sets.data());
		if (result != VK_SUCCESS)
			throw std::runtime_error("failed to allocate descriptor sets!");

		std::vector<VkDescriptorBufferInfo> buffer_infos;
		std::vector<VkDescriptorImageInfo> image_infos;
		for (size_t frame = 0; frame < count; frame++)
		{
			for (size_t binding = 0; binding < uniform_sizes.size(); ++binding)
			{
				buffer_infos.emplace_back(VkDescriptorBufferInfo{
					.buffer = uniform_buffers[frame][binding],
					.offset = 0,
					.range = uniform_sizes[binding] // or VK_WHOLE_SIZE
					});
			}

			if (texture != nullptr)
			{
				image_infos.emplace_back(VkDescriptorImageInfo{
					.sampler = texture->GetSampler(),
					.imageView = texture->GetImageView(),
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					});
			}
		}

		std::vector<VkWriteDescriptorSet> descriptor_writes;
		for (size_t frame = 0; frame < count; ++frame)
		{
			for (size_t binding = 0; binding < uniform_sizes.size(); ++binding)
			{
				descriptor_writes.emplace_back(VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = descriptor_sets[frame],
					.dstBinding = static_cast<std::uint32_t>(binding),
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pImageInfo = nullptr,
					.pBufferInfo = &buffer_infos[descriptor_writes.size()],
					.pTexelBufferView = nullptr
				});
			}
		}
		if (texture != nullptr)
		{
			for (size_t frame = 0; frame < count; ++frame)
			{
				descriptor_writes.emplace_back(VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = descriptor_sets[frame],
					.dstBinding = static_cast<std::uint32_t>(uniform_sizes.size()),
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &image_infos[frame],
					.pBufferInfo = nullptr,
					.pTexelBufferView = nullptr
					});
			}
		}

		vkUpdateDescriptorSets(device,
			static_cast<std::uint32_t>(descriptor_writes.size()),
			descriptor_writes.data(),
			0 /*descriptorCopyCount*/,
			nullptr);

		return descriptor_sets;
	}

	void create_uniform_buffer(
		GraphicsApi const & graphics_api,
		VkDeviceSize buffer_size,
		VkBuffer & out_uniform_buffer,
		VkDeviceMemory & out_buffer_memory,
		void *& out_mapped_buffer)
	{
		graphics_api.CreateBuffer(
			buffer_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			out_uniform_buffer,
			out_buffer_memory);

		vkMapMemory(
			graphics_api.GetDevice(),
			out_buffer_memory,
			0 /*offset*/,
			buffer_size,
			0 /*flags*/,
			&out_mapped_buffer);
	}
}

GraphicsPipeline::GraphicsPipeline(GraphicsApi const & graphics_api,
	VkShaderModule vert_shader_module,
	VkShaderModule frag_shader_module,
	VkVertexInputBindingDescription const & binding_desc,
	std::vector<VkVertexInputAttributeDescription> const & attrib_descs,
	std::vector<VkPushConstantRange> push_constants_ranges,
	std::vector<VkDeviceSize> vs_uniform_sizes,
	std::vector<VkDeviceSize> fs_uniform_sizes,
	Texture const * texture,
	DepthTestOptions const & depth_options,
	BlendOptions const & blend_options,
	CullMode cull_mode,
	PerFrameConstantsCallback per_frame_constants_callback,
	PerObjectConstantsCallback per_object_constants_callback)
	: m_graphics_api(graphics_api)
	, m_per_frame_constants_callback(per_frame_constants_callback)
	, m_per_object_constants_callback(per_object_constants_callback)
{
	VkDevice device = m_graphics_api.GetDevice();

	m_descriptor_set_layout = create_descriptor_set_layout(device,
		static_cast<std::uint32_t>(vs_uniform_sizes.size()),
		static_cast<std::uint32_t>(fs_uniform_sizes.size()),
		texture != nullptr);

	std::vector<VkDeviceSize> uniform_sizes{ vs_uniform_sizes };
	uniform_sizes.insert(uniform_sizes.end(), fs_uniform_sizes.begin(), fs_uniform_sizes.end());

	m_descriptor_pool = create_descriptor_pool(device,
		static_cast<std::uint32_t>(uniform_sizes.size()) /*descriptor_count*/,
		GraphicsApi::m_max_frames_in_flight /*descriptor_set_count*/,
		texture != nullptr);

	std::array<std::vector<VkBuffer>, GraphicsApi::m_max_frames_in_flight> uniform_buffers;
	for (size_t frame = 0; frame < GraphicsApi::m_max_frames_in_flight; ++frame)
	{
		m_descriptor_sets[frame].m_uniform_buffers.resize(uniform_sizes.size());
		for (int binding = 0; binding < uniform_sizes.size(); ++binding)
		{
			UniformBuffer & uniform = m_descriptor_sets[frame].m_uniform_buffers[binding];
			create_uniform_buffer(m_graphics_api, uniform_sizes[binding],
				uniform.m_buffer, uniform.m_memory, uniform.m_mapping);
			uniform_buffers[frame].push_back(uniform.m_buffer);
		}
	}

	std::array<VkDescriptorSet, GraphicsApi::m_max_frames_in_flight> descriptor_sets
		= create_descriptor_sets<GraphicsApi::m_max_frames_in_flight>(device,
			m_descriptor_set_layout, m_descriptor_pool, uniform_buffers, uniform_sizes, texture);

	for (size_t frame = 0; frame < GraphicsApi::m_max_frames_in_flight; ++frame)
		m_descriptor_sets[frame].m_descriptor_set = descriptor_sets[frame];

	m_pipeline_layout = create_pipeline_layout(device, m_descriptor_set_layout, push_constants_ranges);
	if (m_pipeline_layout == VK_NULL_HANDLE)
		return;

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

	m_graphics_pipeline = create_graphics_pipeline(
		device,
		m_graphics_api.GetRenderPass(),
		m_pipeline_layout,
		shader_stages,
		binding_desc,
		attrib_descs,
		depth_options,
		blend_options,
		cull_mode);
}

GraphicsPipeline::~GraphicsPipeline()
{
	destroy_pipeline();
}

void GraphicsPipeline::destroy_pipeline()
{
	VkDevice device = m_graphics_api.GetDevice();

	for (DescriptorSet & descriptor_set : m_descriptor_sets)
	{
		for (UniformBuffer & uniform : descriptor_set.m_uniform_buffers)
		{
			vkDestroyBuffer(device, uniform.m_buffer, nullptr);
			vkFreeMemory(device, uniform.m_memory, nullptr);
		}
	}

	vkDestroyDescriptorPool(device, m_descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(device, m_descriptor_set_layout, nullptr);

	vkDestroyPipeline(device, m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline && other)
	: m_graphics_api(other.m_graphics_api)
{
	*this = std::move(other);
}

GraphicsPipeline & GraphicsPipeline::operator=(GraphicsPipeline && other)
{
	if (this == &other)
		return *this;

	destroy_pipeline();

	m_graphics_pipeline = other.m_graphics_pipeline;
	m_pipeline_layout = other.m_pipeline_layout;
	m_descriptor_set_layout = other.m_descriptor_set_layout;
	m_descriptor_pool = other.m_descriptor_pool;
	m_descriptor_sets = std::move(other.m_descriptor_sets);
	m_per_frame_constants_callback = other.m_per_frame_constants_callback;
	m_per_object_constants_callback = other.m_per_object_constants_callback;

	other.m_graphics_pipeline = VK_NULL_HANDLE;
	other.m_pipeline_layout = VK_NULL_HANDLE;
	other.m_descriptor_set_layout = VK_NULL_HANDLE;
	other.m_descriptor_pool = VK_NULL_HANDLE;
	other.m_descriptor_sets.fill(DescriptorSet{});
	other.m_per_frame_constants_callback = nullptr;
	other.m_per_object_constants_callback = nullptr;

	return *this;
}

void GraphicsPipeline::Activate() const
{
	if (m_graphics_pipeline == VK_NULL_HANDLE)
	{
		std::cout << "Activating invalid graphics pipeline" << std::endl;
		return;
	}

	VkCommandBuffer command_buffer = m_graphics_api.GetCurCommandBuffer();

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

	vkCmdBindDescriptorSets(command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipeline_layout,
		0 /*firstSet*/,
		1 /*descriptor_set_count*/,
		&m_descriptor_sets[m_graphics_api.GetCurFrameIndex()].m_descriptor_set,
		0 /*dynamicOffsetCount*/,
		nullptr);
}

void GraphicsPipeline::UpdatePerFrameConstants() const
{
	if (m_per_frame_constants_callback)
		m_per_frame_constants_callback(*this);
}

void GraphicsPipeline::UpdatePerObjectConstants(RenderObject const & obj) const
{
	if (m_per_object_constants_callback)
		m_per_object_constants_callback(*this, obj);
}
