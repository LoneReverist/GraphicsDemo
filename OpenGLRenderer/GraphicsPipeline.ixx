// GraphicsPipeline.ixx

module;

#include <filesystem>
#include <functional>
#include <iostream>

#include <glad/glad.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

export module GraphicsPipeline;

import RenderObject;

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

export class GraphicsPipeline
{
public:
	using PerFrameConstantsCallback = std::function<void(GraphicsPipeline const & pipeline)>;
	using PerObjectConstantsCallback = std::function<void(GraphicsPipeline const & pipeline, RenderObject const &)>;

	GraphicsPipeline(
		unsigned int vert_shader_id,
		unsigned int frag_shader_id,
		DepthTestOptions const & depth_options,
		PerFrameConstantsCallback per_frame_constants_callback,
		PerObjectConstantsCallback per_object_constants_callback);
	~GraphicsPipeline();

	GraphicsPipeline(GraphicsPipeline && other);
	GraphicsPipeline & operator=(GraphicsPipeline && other);

	GraphicsPipeline(GraphicsPipeline &) = delete;
	GraphicsPipeline & operator=(GraphicsPipeline &) = delete;

	bool IsValid() const { return m_program_id != 0; }

	void Activate() const;
	void UpdatePerFrameConstants() const;
	void UpdatePerObjectConstants(RenderObject const & obj) const;

	template <typename T>
	void SetUniform(std::string const & label, T const & data) const;

private:
	void destroy_pipeline();

private:
	unsigned int m_program_id = 0;

	DepthTestOptions m_depth_test_options;

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <typename T>
void GraphicsPipeline::SetUniform(std::string const & label, T const & data) const
{
	GLint uniform_loc = glGetUniformLocation(m_program_id, label.c_str());
	if (uniform_loc == -1)
	{
		std::cout << "Uniform not found: " << label << std::endl;
		return;
	}

	if constexpr (std::same_as<T, float>)
		glUniform1fv(uniform_loc, 1, &data);
	else if constexpr (std::same_as<T, glm::vec2>)
		glUniform2fv(uniform_loc, 1, glm::value_ptr(data));
	else if constexpr (std::same_as<T, glm::vec3>)
		glUniform3fv(uniform_loc, 1, glm::value_ptr(data));
	else if constexpr (std::same_as<T, glm::vec4>)
		glUniform4fv(uniform_loc, 1, glm::value_ptr(data));
	else if constexpr (std::same_as<T, glm::mat4>)
		glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(data));
	else
		static_assert(false, "Unsupported uniform type");
}
