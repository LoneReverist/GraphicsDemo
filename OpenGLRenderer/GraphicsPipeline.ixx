// GraphicsPipeline.ixx

module;

#include <filesystem>
#include <functional>
#include <iostream>

#include <glad/glad.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

export module GraphicsPipeline;

import Buffer;
import RenderObject;

struct UniformBuffer
{
	Buffer m_buffer;
	size_t m_size = 0;
};

struct DescriptorSet
{
	std::vector<UniformBuffer> m_uniform_buffers;
};

export enum class DepthCompareOp
{
	NEVER = GL_NEVER,
	LESS = GL_LESS,
	EQUAL = GL_EQUAL,
	LESS_OR_EQUAL = GL_LEQUAL,
	GREATER = GL_GREATER,
	NOT_EQUAL = GL_NOTEQUAL,
	GREATER_OR_EQUAL = GL_GEQUAL,
	ALWAYS = GL_ALWAYS
};

export struct DepthTestOptions
{
	bool m_enable_depth_test = true;
	bool m_enable_depth_write = true;
	DepthCompareOp m_depth_compare_op = DepthCompareOp::LESS;
};

export enum class BlendFactor {
	ZERO = GL_ZERO,
	ONE = GL_ONE,
	SRC_COLOR = GL_SRC_COLOR,
	ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
	DST_COLOR = GL_DST_COLOR,
	ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
	SRC_ALPHA = GL_SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = GL_DST_ALPHA,
	ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
	CONSTANT_COLOR = GL_CONSTANT_COLOR,
	ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
	CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
	ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
	SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
};

export struct BlendOptions
{
	bool m_enable_blend = false;
	BlendFactor m_src_factor = BlendFactor::SRC_ALPHA;
	BlendFactor m_dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA;
};

// By default, front facing facets have counter-clockwise vertex windings.
export enum class CullMode
{
	NONE = GL_NONE,
	FRONT = GL_FRONT,
	BACK = GL_BACK
};

class Program
{
public:
	Program() = default;
	~Program();

	Program(Program && other) noexcept;
	Program & operator=(Program && other) noexcept;

	Program(Program const &) = delete;
	Program & operator=(Program const &) = delete;

	void Create();

	unsigned int GetId() const { return m_id; }

private:
	unsigned int m_id = 0;
};

export class GraphicsPipeline
{
public:
	using PerFrameConstantsCallback = std::function<void(GraphicsPipeline const & pipeline)>;
	using PerObjectConstantsCallback = std::function<void(GraphicsPipeline const & pipeline, RenderObject const &)>;

	GraphicsPipeline(
		unsigned int vert_shader_id,
		unsigned int frag_shader_id,
		std::vector<size_t> vs_uniform_sizes,
		std::vector<size_t> fs_uniform_sizes,
		DepthTestOptions const & depth_options,
		BlendOptions const & blend_options,
		CullMode cull_mode,
		PerFrameConstantsCallback per_frame_constants_callback,
		PerObjectConstantsCallback per_object_constants_callback);
	~GraphicsPipeline() = default;

	GraphicsPipeline(GraphicsPipeline && other) = default;
	GraphicsPipeline & operator=(GraphicsPipeline && other) = default;

	GraphicsPipeline(GraphicsPipeline const &) = delete;
	GraphicsPipeline & operator=(GraphicsPipeline const &) = delete;

	bool IsValid() const { return m_program.GetId() != 0; }

	void Activate() const;
	void UpdatePerFrameConstants() const;
	void UpdatePerObjectConstants(RenderObject const & obj) const;

	template <typename UniformData>
	void SetUniform(std::string const & label, UniformData const & data) const;

	template <typename UniformData>
	void SetUniform(std::uint32_t binding, UniformData const & data) const;

private:
	void destroy_pipeline();

private:
	Program m_program;

	DescriptorSet m_descriptor_set;

	DepthTestOptions m_depth_test_options;
	BlendOptions m_blend_options;
	CullMode m_cull_mode = CullMode::NONE;

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <typename UniformData>
void GraphicsPipeline::SetUniform(std::string const & label, UniformData const & data) const
{
	GLint uniform_loc = glGetUniformLocation(m_program.GetId(), label.c_str());
	if (uniform_loc == -1)
	{
		std::cout << "Uniform not found: " << label << std::endl;
		return;
	}

	if constexpr (std::same_as<UniformData, float>)
		glUniform1fv(uniform_loc, 1, &data);
	else if constexpr (std::same_as<UniformData, glm::vec2>)
		glUniform2fv(uniform_loc, 1, glm::value_ptr(data));
	else if constexpr (std::same_as<UniformData, glm::vec3>)
		glUniform3fv(uniform_loc, 1, glm::value_ptr(data));
	else if constexpr (std::same_as<UniformData, glm::vec4>)
		glUniform4fv(uniform_loc, 1, glm::value_ptr(data));
	else if constexpr (std::same_as<UniformData, glm::mat4>)
		glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(data));
	else
		static_assert(false, "Unsupported uniform type");
}

template <typename UniformData>
void GraphicsPipeline::SetUniform(std::uint32_t binding, UniformData const & data) const
{
	if (binding >= m_descriptor_set.m_uniform_buffers.size())
	{
		std::cout << "Invalid uniform binding: " << binding << std::endl;
		return;
	}
	UniformBuffer const & uniform = m_descriptor_set.m_uniform_buffers[binding];
	if (uniform.m_buffer.GetId() == 0)
	{
		std::cout << "Uniform buffer not initialized for binding: " << binding << std::endl;
		return;
	}
	if (uniform.m_size != sizeof(data))
	{
		std::cout << "Uniform buffer size is different from data size: " << binding << std::endl;
		return;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, uniform.m_buffer.GetId());
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, uniform.m_buffer.GetId());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
