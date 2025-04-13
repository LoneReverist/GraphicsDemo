// GraphicsPipeline.cpp

module;

#include <fstream>
#include <iostream>

#include <glad/glad.h>

module GraphicsPipeline;

GraphicsPipeline::GraphicsPipeline(
	unsigned int vert_shader_id,
	unsigned int frag_shader_id,
	DepthTestOptions const & depth_options,
	PerFrameConstantsCallback per_frame_constants_callback,
	PerObjectConstantsCallback per_object_constants_callback)
	: m_depth_test_options(depth_options)
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

	m_program_id = other.m_program_id;
	m_depth_test_options = other.m_depth_test_options;
	m_per_frame_constants_callback = other.m_per_frame_constants_callback;
	m_per_object_constants_callback = other.m_per_object_constants_callback;

	other.m_program_id = 0;
	other.m_depth_test_options = DepthTestOptions{};
	other.m_per_frame_constants_callback = nullptr;
	other.m_per_object_constants_callback = nullptr;

	return *this;
}

void GraphicsPipeline::Activate() const
{
	if (m_program_id == 0)
	{
		std::cout << "Activating invalid shader program" << std::endl;
		return;
	}

	glEnable(GL_CULL_FACE); // cull back facing facets. by default, front facing facets have counter-clockwise vertex windings.

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
