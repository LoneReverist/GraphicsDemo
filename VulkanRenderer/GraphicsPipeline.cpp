// GraphicsPipeline.cpp

module;

#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

module GraphicsPipeline;

import GraphicsApi;
import GraphicsError;

DescriptorSets::DescriptorSets(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

vk::raii::DescriptorSetLayout create_descriptor_set_layout(
	vk::raii::Device const & device,
	std::uint32_t vs_descriptor_set_count,
	std::uint32_t fs_descriptor_set_count,
	bool has_texture)
{
	std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
	layout_bindings.reserve(vs_descriptor_set_count + fs_descriptor_set_count + (has_texture ? 1 : 0));

	for (std::uint32_t i = 0; i < vs_descriptor_set_count; ++i)
	{
		layout_bindings.emplace_back(
			vk::DescriptorSetLayoutBinding{
				.binding = static_cast<std::uint32_t>(layout_bindings.size()),
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eVertex,
				.pImmutableSamplers = nullptr
			});
	}
	for (std::uint32_t i = 0; i < fs_descriptor_set_count; ++i)
	{
		layout_bindings.emplace_back(
			vk::DescriptorSetLayoutBinding{
				.binding = static_cast<std::uint32_t>(layout_bindings.size()),
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment,
				.pImmutableSamplers = nullptr
			});
	}

	if (has_texture)
	{
		layout_bindings.emplace_back(
			vk::DescriptorSetLayoutBinding{
				.binding = static_cast<std::uint32_t>(layout_bindings.size()),
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment,
				.pImmutableSamplers = nullptr
			});
	}

	vk::DescriptorSetLayoutCreateInfo layout_info{
		.bindingCount = static_cast<std::uint32_t>(layout_bindings.size()),
		.pBindings = layout_bindings.data()
	};

	return vk::raii::DescriptorSetLayout{ device, layout_info };
}

vk::raii::DescriptorPool create_descriptor_pool(
	vk::raii::Device const & device,
	std::uint32_t uniform_count,
	std::uint32_t descriptor_set_count,
	bool has_texture)
{
	std::vector<vk::DescriptorPoolSize> pool_sizes;
	if (uniform_count > 0)
	{
		pool_sizes.emplace_back(
			vk::DescriptorPoolSize{
				.type = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = uniform_count * descriptor_set_count
			});
	}

	if (has_texture)
	{
		pool_sizes.emplace_back(
			vk::DescriptorPoolSize{
				.type = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = descriptor_set_count
			});
	}

	vk::DescriptorPoolCreateInfo pool_info{
		.maxSets = descriptor_set_count,
		.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
		.pPoolSizes = pool_sizes.data()
	};

	return vk::raii::DescriptorPool{ device, pool_info };
}

template <std::uint32_t count>
std::vector<vk::raii::DescriptorSet> create_descriptor_sets(
	vk::raii::Device const & device,
	vk::raii::DescriptorSetLayout const & layout,
	vk::DescriptorPool pool,
	std::array<std::vector<vk::Buffer>, count> uniform_buffers,
	std::vector<vk::DeviceSize> uniform_sizes,
	Texture const * texture)
{
	std::array<vk::DescriptorSetLayout, count> layouts;
	layouts.fill(layout);

	vk::DescriptorSetAllocateInfo alloc_info{
		.descriptorPool = pool,
		.descriptorSetCount = static_cast<std::uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data()
	};

	std::vector<vk::raii::DescriptorSet> descriptor_sets = device.allocateDescriptorSets(alloc_info);

	std::vector<vk::DescriptorBufferInfo> buffer_infos;
	std::vector<vk::DescriptorImageInfo> image_infos;
	for (size_t frame = 0; frame < count; frame++)
	{
		for (size_t binding = 0; binding < uniform_sizes.size(); ++binding)
		{
			buffer_infos.emplace_back(vk::DescriptorBufferInfo{
				.buffer = uniform_buffers[frame][binding],
				.offset = 0,
				.range = uniform_sizes[binding] // or VK_WHOLE_SIZE
				});
		}

		if (texture != nullptr && texture->IsValid())
		{
			image_infos.emplace_back(vk::DescriptorImageInfo{
				.sampler = *texture->GetSampler(),
				.imageView = *texture->GetImageView(),
				.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
				});
		}
	}

	std::vector<vk::WriteDescriptorSet> descriptor_writes;
	for (size_t frame = 0; frame < count; ++frame)
	{
		for (size_t binding = 0; binding < uniform_sizes.size(); ++binding)
		{
			descriptor_writes.emplace_back(vk::WriteDescriptorSet{
				.dstSet = descriptor_sets[frame],
				.dstBinding = static_cast<std::uint32_t>(binding),
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pImageInfo = nullptr,
				.pBufferInfo = &buffer_infos[descriptor_writes.size()],
				.pTexelBufferView = nullptr
				});
		}
	}
	if (texture != nullptr && texture->IsValid())
	{
		for (size_t frame = 0; frame < count; ++frame)
		{
			descriptor_writes.emplace_back(vk::WriteDescriptorSet{
				.dstSet = descriptor_sets[frame],
				.dstBinding = static_cast<std::uint32_t>(uniform_sizes.size()),
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &image_infos[frame],
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
				});
		}
	}

	device.updateDescriptorSets(descriptor_writes, {});

	return descriptor_sets;
}

void create_uniform_buffer(
	GraphicsApi const & graphics_api,
	vk::DeviceSize buffer_size,
	UniformBuffer & out_uniform_buffer)
{
	out_uniform_buffer.buffer.Create(
		graphics_api,
		buffer_size,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	out_uniform_buffer.mapping = out_uniform_buffer.buffer.GetMemory().mapMemory(0 /*offset*/, buffer_size);
}

void DescriptorSets::Create(
	std::vector<vk::DeviceSize> const & vs_uniform_sizes,
	std::vector<vk::DeviceSize> const & fs_uniform_sizes,
	Texture const * texture)
{
	vk::raii::Device const & device = m_graphics_api.get().GetDevice();
	bool has_texture = texture != nullptr && texture->IsValid();

	m_descriptor_set_layout = create_descriptor_set_layout(device,
		static_cast<std::uint32_t>(vs_uniform_sizes.size()),
		static_cast<std::uint32_t>(fs_uniform_sizes.size()),
		has_texture);

	std::vector<VkDeviceSize> uniform_sizes{ vs_uniform_sizes };
	uniform_sizes.insert(uniform_sizes.end(), fs_uniform_sizes.begin(), fs_uniform_sizes.end());

	m_descriptor_pool = create_descriptor_pool(device,
		static_cast<std::uint32_t>(uniform_sizes.size()) /*descriptor_count*/,
		GraphicsApi::m_max_frames_in_flight /*descriptor_set_count*/,
		has_texture);

	std::array<std::vector<vk::Buffer>, GraphicsApi::m_max_frames_in_flight> uniform_buffers;
	for (size_t frame = 0; frame < GraphicsApi::m_max_frames_in_flight; ++frame)
	{
		m_descriptor_sets[frame].uniform_buffers.reserve(uniform_sizes.size());
		for (int binding = 0; binding < uniform_sizes.size(); ++binding)
		{
			UniformBuffer & uniform = m_descriptor_sets[frame].uniform_buffers.emplace_back();
			create_uniform_buffer(m_graphics_api, uniform_sizes[binding], uniform);
			uniform_buffers[frame].push_back(*uniform.buffer.Get());
		}
	}

	std::vector<vk::raii::DescriptorSet> descriptor_sets = create_descriptor_sets<GraphicsApi::m_max_frames_in_flight>(
		device, m_descriptor_set_layout, m_descriptor_pool, uniform_buffers, uniform_sizes, texture);

	for (size_t frame = 0; frame < GraphicsApi::m_max_frames_in_flight; ++frame)
		m_descriptor_sets[frame].descriptor_set = std::move(descriptor_sets[frame]);
}

vk::raii::PipelineLayout create_pipeline_layout(
	vk::raii::Device const & device,
	vk::DescriptorSetLayout descriptor_set_layout,
	std::vector<vk::PushConstantRange> const & push_constant_ranges)
{
	vk::PipelineLayoutCreateInfo pipeline_layout_info{
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_set_layout,
		.pushConstantRangeCount = static_cast<std::uint32_t>(push_constant_ranges.size()),
		.pPushConstantRanges = push_constant_ranges.data(),
	};
	return vk::raii::PipelineLayout(device, pipeline_layout_info);
}

vk::raii::Pipeline create_pipeline(
	vk::raii::Device const & device,
	vk::raii::PipelineLayout const & pipeline_layout,
	std::vector<vk::PipelineShaderStageCreateInfo> const & shader_stages,
	vk::VertexInputBindingDescription const & binding_desc,
	std::vector<vk::VertexInputAttributeDescription> const & attrib_descs,
	vk::Format swapchain_image_format,
	vk::Format depth_image_format,
	DepthTestOptions const & depth_options,
	BlendOptions const & blend_options,
	CullMode cull_mode)
{
	std::vector<vk::DynamicState> dynamic_states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamic_state{
		.dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data()
	};

	vk::PipelineViewportStateCreateInfo viewport_state{
		.viewportCount = 1,
		.scissorCount = 1
	};

	vk::PipelineVertexInputStateCreateInfo vertex_input_info{
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding_desc,
		.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attrib_descs.size()),
		.pVertexAttributeDescriptions = attrib_descs.data()
	};

	vk::PipelineInputAssemblyStateCreateInfo input_assembly{
		.topology = vk::PrimitiveTopology::eTriangleList,
		.primitiveRestartEnable = vk::False
	};

	vk::PipelineRasterizationStateCreateInfo rasterizer{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = static_cast<vk::CullModeFlagBits>(cull_mode),
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthBiasEnable = vk::False,
		.lineWidth = 1.0f
	};

	vk::PipelineMultisampleStateCreateInfo multisampling{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};

	vk::PipelineColorBlendAttachmentState color_blend_attachment{
		.blendEnable = blend_options.enable_blend ? vk::True : vk::False,
		.srcColorBlendFactor = static_cast<vk::BlendFactor>(blend_options.src_factor),
		.dstColorBlendFactor = static_cast<vk::BlendFactor>(blend_options.dst_factor),
		.colorBlendOp = vk::BlendOp::eAdd,
		.srcAlphaBlendFactor = static_cast<vk::BlendFactor>(blend_options.src_factor),
		.dstAlphaBlendFactor = static_cast<vk::BlendFactor>(blend_options.dst_factor),
		.alphaBlendOp = vk::BlendOp::eAdd,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	vk::PipelineColorBlendStateCreateInfo color_blending{ // global blending options
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment
	};

	vk::PipelineDepthStencilStateCreateInfo depth_stencil{
		.depthTestEnable = depth_options.enable_depth_test ? vk::True : vk::False,
		.depthWriteEnable = depth_options.enable_depth_write ? vk::True : vk::False,
		.depthCompareOp = static_cast<vk::CompareOp>(depth_options.depth_compare_op),
		.depthBoundsTestEnable = vk::False,
		.stencilTestEnable = vk::False
	};

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_info{
		{
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
			.renderPass = nullptr,
		},
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &swapchain_image_format,
			.depthAttachmentFormat = depth_image_format
		}
	};
	
	return vk::raii::Pipeline(device, nullptr, pipeline_info.get<vk::GraphicsPipelineCreateInfo>());
}

GraphicsPipeline::GraphicsPipeline(GraphicsApi const & graphics_api,
	PerFrameConstantsCallback per_frame_constants_callback,
	PerObjectConstantsCallback per_object_constants_callback)
	: m_graphics_api(graphics_api)
	, m_descriptor_sets(graphics_api)
	, m_per_frame_constants_callback(per_frame_constants_callback)
	, m_per_object_constants_callback(per_object_constants_callback)
{
}

std::expected<void, GraphicsError> GraphicsPipeline::Create(
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stages,
	vk::VertexInputBindingDescription const & binding_desc,
	std::vector<vk::VertexInputAttributeDescription> const & attrib_descs,
	std::vector<vk::PushConstantRange> const & push_constants_ranges,
	std::vector<vk::DeviceSize> const & vs_uniform_sizes,
	std::vector<vk::DeviceSize> const & fs_uniform_sizes,
	Texture const * texture,
	DepthTestOptions const & depth_options,
	BlendOptions const & blend_options,
	CullMode cull_mode)
{
	try
	{
		m_descriptor_sets.Create(vs_uniform_sizes, fs_uniform_sizes, texture);

		m_pipeline_layout = create_pipeline_layout(
			m_graphics_api.get().GetDevice(),
			m_descriptor_sets.GetLayout(),
			push_constants_ranges);

		m_pipeline = create_pipeline(
			m_graphics_api.get().GetDevice(),
			m_pipeline_layout,
			shader_stages,
			binding_desc,
			attrib_descs,
			m_graphics_api.get().GetSwapChainImageFormat(),
			m_graphics_api.get().GetDepthImageFormat(),
			depth_options,
			blend_options,
			cull_mode);
	}
	catch (vk::SystemError const & err)
	{
		return std::unexpected{ GraphicsError{ "Vulkan error: " + std::string(err.what()) } };
	}

	return {};
}

void GraphicsPipeline::Activate() const
{
	if (m_pipeline == nullptr)
		return;

	vk::raii::CommandBuffer const & command_buffer = m_graphics_api.get().GetCurCommandBuffer();

	command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);

	command_buffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		*m_pipeline_layout,
		0 /*firstSet*/,
		vk::DescriptorSet{ m_descriptor_sets.GetCurrent().descriptor_set },
		{} /*dynamicOffsets*/
	);
}

void GraphicsPipeline::UpdatePerFrameConstants() const
{
	if (m_per_frame_constants_callback)
		m_per_frame_constants_callback(*this);
}

void GraphicsPipeline::UpdatePerObjectConstants(void const * object_data) const
{
	if (m_per_object_constants_callback)
		m_per_object_constants_callback(*this, object_data);
}
