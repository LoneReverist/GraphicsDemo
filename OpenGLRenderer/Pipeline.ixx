// Pipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include <glad/glad.h>

export module Dreamhearth:Pipeline;

import :Buffer;
import :GraphicsError;
import :Texture;

namespace Dreamhearth
{
	struct UniformBuffer
	{
		Buffer buffer;
		size_t size = 0;
	};

	struct DescriptorSet
	{
		std::vector<UniformBuffer> uniform_buffers;
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
		bool uses_depth_attachment{ true }; // For vulkan compatibility. This specifies whether the render target has a depth buffer or not.
		bool enable_depth_test = true;
		bool enable_depth_write = true;
		DepthCompareOp depth_compare_op = DepthCompareOp::LESS;
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
		bool enable_blend = false;
		BlendFactor src_factor = BlendFactor::SRC_ALPHA;
		BlendFactor dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA;
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

	export class Pipeline
	{
	public:
		using PerFrameConstantsCallback = std::function<void(Pipeline const & pipeline)>;
		using PerObjectConstantsCallback = std::function<void(Pipeline const & pipeline, void const * object_data)>;

		explicit Pipeline(
			RenderContext const & render_context,
			PerFrameConstantsCallback per_frame_constants_callback,
			PerObjectConstantsCallback per_object_constants_callback);
		~Pipeline() = default;

		Pipeline(Pipeline && other) = default;
		Pipeline & operator=(Pipeline && other) = default;

		Pipeline(Pipeline const &) = delete;
		Pipeline & operator=(Pipeline const &) = delete;

		std::expected<void, GraphicsError> Create(
			unsigned int vert_shader_id,
			unsigned int frag_shader_id,
			size_t vs_object_uniform_size,
			size_t fs_object_uniform_size,
			std::vector<size_t> vs_uniform_sizes,
			std::vector<size_t> fs_uniform_sizes,
			bool has_texture,
			DepthTestOptions const & depth_options,
			BlendOptions const & blend_options,
			CullMode cull_mode);

		bool IsValid() const { return m_program.GetId() != 0; }

		void Activate() const;
		void UpdatePerFrameConstants() const;
		void UpdatePerObjectConstants(void const * object_data) const;
		void BindTexture(std::uint32_t binding, Texture const & texture) const;

		template <typename UniformData>
		void SetUniform(std::uint32_t binding, UniformData const & data) const;

		template <typename ObjectDataVS = std::nullopt_t, typename ObjectDataFS = std::nullopt_t>
		void SetObjectData(ObjectDataVS const & vs_data, ObjectDataFS const & fs_data) const;

	private:
		std::reference_wrapper<RenderContext const> m_render_context;

		Program m_program;

		DescriptorSet m_descriptor_set;
		UniformBuffer m_vs_object_uniform;
		UniformBuffer m_fs_object_uniform;

		DepthTestOptions m_depth_test_options;
		BlendOptions m_blend_options;
		CullMode m_cull_mode = CullMode::NONE;

		PerFrameConstantsCallback m_per_frame_constants_callback;
		PerObjectConstantsCallback m_per_object_constants_callback;
	};

	template <typename UniformData>
	void set_uniform(
		RenderContext const & render_context,
		std::uint32_t binding,
		UniformBuffer const & uniform,
		UniformData const & data)
	{
		if (uniform.buffer.GetId() == 0)
		{
			render_context.ReportDiagnostic({
				.severity = GraphicsDiagnosticSeverity::Error,
				.message = "Uniform buffer not initialized for binding: " + std::to_string(binding)
			});
			return;
		}
		if (uniform.size != sizeof(data))
		{
			render_context.ReportDiagnostic({
				.severity = GraphicsDiagnosticSeverity::Error,
				.message = "Uniform buffer size differs from data size for binding: " + std::to_string(binding)
			});
			return;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, uniform.buffer.GetId());
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, uniform.buffer.GetId());
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	template <typename UniformData>
	void Pipeline::SetUniform(std::uint32_t binding, UniformData const & data) const
	{
		if (binding >= m_descriptor_set.uniform_buffers.size())
		{
			m_render_context.get().ReportDiagnostic({
				.severity = GraphicsDiagnosticSeverity::Error,
				.message = "Invalid uniform binding: " + std::to_string(binding)
			});
			return;
		}

		UniformBuffer const & uniform = m_descriptor_set.uniform_buffers[binding];
		set_uniform(m_render_context.get(), binding, uniform, data);
	}

	template <typename ObjectDataVS /*= std::nullopt_t*/, typename ObjectDataFS /*= std::nullopt_t*/>
	void Pipeline::SetObjectData(ObjectDataVS const & vs_data, ObjectDataFS const & fs_data) const
	{
		static_assert(!std::same_as<ObjectDataVS, std::nullopt_t> || !std::same_as<ObjectDataFS, std::nullopt_t>,
			"At least one object data must be provided");

		if constexpr (!std::same_as<ObjectDataVS, std::nullopt_t>)
		{
			GLuint blockIndex = glGetUniformBlockIndex(m_program.GetId(), "ObjectDataVS");
			GLint binding = 0;
			glGetActiveUniformBlockiv(m_program.GetId(), blockIndex, GL_UNIFORM_BLOCK_BINDING, &binding);
			set_uniform(m_render_context.get(), binding, m_vs_object_uniform, vs_data);
		}

		if constexpr (!std::same_as<ObjectDataFS, std::nullopt_t>)
		{
			GLuint blockIndex = glGetUniformBlockIndex(m_program.GetId(), "ObjectDataFS");
			GLint binding = 0;
			glGetActiveUniformBlockiv(m_program.GetId(), blockIndex, GL_UNIFORM_BLOCK_BINDING, &binding);
			set_uniform(m_render_context.get(), binding, m_fs_object_uniform, fs_data);
		}
	}
} // namespace Dreamhearth
