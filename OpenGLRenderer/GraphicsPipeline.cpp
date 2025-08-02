// GraphicsPipeline.cpp

module;

#include <fstream>
#include <iostream>

#include <glad/glad.h>

module GraphicsPipeline;

GraphicsPipeline::GraphicsPipeline(
	unsigned int vert_shader_id,
	unsigned int frag_shader_id,
	std::vector<size_t> vs_uniform_sizes,
	std::vector<size_t> fs_uniform_sizes,
	DepthTestOptions const & depth_options,
	BlendOptions const & blend_options,
	CullMode cull_mode,
	PerFrameConstantsCallback per_frame_constants_callback,
	PerObjectConstantsCallback per_object_constants_callback)
	: m_depth_test_options(depth_options)
	, m_blend_options(blend_options)
	, m_cull_mode(cull_mode)
	, m_per_frame_constants_callback(per_frame_constants_callback)
	, m_per_object_constants_callback(per_object_constants_callback)
{
	m_program_id = glCreateProgram();
	glAttachShader(m_program_id, vert_shader_id);
	glAttachShader(m_program_id, frag_shader_id);
	glLinkProgram(m_program_id);

	int success = 0;
	glGetProgramiv(m_program_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char info_log[512];
		glGetProgramInfoLog(m_program_id, 512, nullptr, info_log);
		std::cout << "Failed to link shader program:\n" << info_log << std::endl;
	}

	std::vector<size_t> uniform_sizes{ vs_uniform_sizes };
	uniform_sizes.insert(uniform_sizes.end(), fs_uniform_sizes.begin(), fs_uniform_sizes.end());

	for (size_t binding = 0; binding < uniform_sizes.size(); ++binding)
	{
		unsigned int buffer_id = 0;
		glGenBuffers(1, &buffer_id);
		glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
		glBufferData(GL_UNIFORM_BUFFER, uniform_sizes[binding], nullptr, GL_DYNAMIC_DRAW);
		m_descriptor_set.m_uniform_buffers.emplace_back(buffer_id, uniform_sizes[binding]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

GraphicsPipeline::~GraphicsPipeline()
{
	destroy_pipeline();
}

void GraphicsPipeline::destroy_pipeline()
{
	glDeleteProgram(m_program_id);
	m_program_id = 0;
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline && other)
{
	*this = std::move(other);
}

GraphicsPipeline & GraphicsPipeline::operator=(GraphicsPipeline && other)
{
	if (this == &other)
		return *this;

	destroy_pipeline();

	std::swap(m_program_id, other.m_program_id);
	std::swap(m_depth_test_options, other.m_depth_test_options);
	std::swap(m_blend_options, other.m_blend_options);
	std::swap(m_cull_mode, other.m_cull_mode);
	std::swap(m_descriptor_set, other.m_descriptor_set);
	std::swap(m_per_frame_constants_callback, other.m_per_frame_constants_callback);
	std::swap(m_per_object_constants_callback, other.m_per_object_constants_callback);
	return *this;
}

void GraphicsPipeline::Activate() const
{
	if (m_program_id == 0)
	{
		std::cout << "Activating invalid shader program" << std::endl;
		return;
	}

	if (m_depth_test_options.m_enable_depth_test)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(m_depth_test_options.m_enable_depth_write ? GL_TRUE : GL_FALSE);
		glDepthFunc(static_cast<GLenum>(m_depth_test_options.m_depth_compare_op));
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (m_blend_options.m_enable_blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(static_cast<GLenum>(m_blend_options.m_src_factor), static_cast<GLenum>(m_blend_options.m_dst_factor));
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (m_cull_mode != CullMode::NONE)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(static_cast<GLenum>(m_cull_mode));
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	glUseProgram(m_program_id);
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
