// GraphicsPipeline.cpp

module;

#include <fstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>

module GraphicsPipeline;

Program::~Program()
{
	if (m_id != 0)
		glDeleteProgram(m_id);
}

Program::Program(Program && other) noexcept
{
	*this = std::move(other);
}

Program & Program::operator=(Program && other) noexcept
{
	if (this != &other)
	{
		if (m_id != 0)
			glDeleteProgram(m_id);

		std::swap(m_id, other.m_id);
	}
	return *this;
}

void Program::Create()
{
	if (m_id != 0)
		return;
	m_id = glCreateProgram();
}

GraphicsPipeline::GraphicsPipeline(
	unsigned int vert_shader_id,
	unsigned int frag_shader_id,
	size_t vs_object_uniform_size,
	size_t fs_object_uniform_size,
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
	m_program.Create();
	glAttachShader(m_program.GetId(), vert_shader_id);
	glAttachShader(m_program.GetId(), frag_shader_id);
	glLinkProgram(m_program.GetId());

	int success = 0;
	glGetProgramiv(m_program.GetId(), GL_LINK_STATUS, &success);
	if (!success)
	{
		char info_log[512];
		glGetProgramInfoLog(m_program.GetId(), 512, nullptr, info_log);
		std::cout << "Failed to link shader program:\n" << info_log << std::endl;
	}

	if (vs_object_uniform_size > 0)
	{
		m_vs_object_uniform.m_buffer.Create();
		m_vs_object_uniform.m_size = vs_object_uniform_size;
		glBindBuffer(GL_UNIFORM_BUFFER, m_vs_object_uniform.m_buffer.GetId());
		glBufferData(GL_UNIFORM_BUFFER, m_vs_object_uniform.m_size, nullptr, GL_DYNAMIC_DRAW);
	}
	if (fs_object_uniform_size > 0)
	{
		m_fs_object_uniform.m_buffer.Create();
		m_fs_object_uniform.m_size = fs_object_uniform_size;
		glBindBuffer(GL_UNIFORM_BUFFER, m_fs_object_uniform.m_buffer.GetId());
		glBufferData(GL_UNIFORM_BUFFER, m_fs_object_uniform.m_size, nullptr, GL_DYNAMIC_DRAW);
	}

	std::vector<size_t> uniform_sizes{ vs_uniform_sizes };
	uniform_sizes.insert(uniform_sizes.end(), fs_uniform_sizes.begin(), fs_uniform_sizes.end());

	m_descriptor_set.m_uniform_buffers.reserve(uniform_sizes.size());
	for (size_t binding = 0; binding < uniform_sizes.size(); ++binding)
	{
		UniformBuffer & uniform = m_descriptor_set.m_uniform_buffers.emplace_back();
		uniform.m_buffer.Create();
		uniform.m_size = uniform_sizes[binding];
		glBindBuffer(GL_UNIFORM_BUFFER, uniform.m_buffer.GetId());
		glBufferData(GL_UNIFORM_BUFFER, uniform.m_size, nullptr, GL_DYNAMIC_DRAW);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void GraphicsPipeline::Activate() const
{
	if (m_program.GetId() == 0)
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

	glUseProgram(m_program.GetId());
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
