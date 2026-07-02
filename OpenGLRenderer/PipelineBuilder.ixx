// PipelineBuilder.ixx

module;

#include <expected>
#include <filesystem>
#include <optional>
#include <vector>

export module Dreamhearth:PipelineBuilder;

import :RenderContext;
import :GraphicsError;
import :Pipeline;
import :Texture;
import :VertexLayout;

namespace Dreamhearth
{
	export class PipelineBuilder
	{
	public:
		using PerFrameConstantsCallback = Pipeline::PerFrameConstantsCallback;
		using PerObjectConstantsCallback = Pipeline::PerObjectConstantsCallback;

		explicit PipelineBuilder(RenderContext const & render_context) : m_render_context(render_context) {}
		~PipelineBuilder();

		std::expected<void, GraphicsError> LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path);

		template <VertexWithLayout VertexT>
		void SetVertexType();

		template <typename InstanceDataT>
		void SetInstanceType();

		template <typename ObjectDataVS = std::nullopt_t, typename ObjectDataFS = std::nullopt_t>
		void SetObjectDataTypes();

		template <typename... UniformTypes>
		void SetVSUniformTypes();

		template <typename... UniformTypes>
		void SetFSUniformTypes();

		void SetHasTexture(bool has_texture) { m_has_texture = has_texture; }
		void SetDepthTestOptions(DepthTestOptions const & options) { m_depth_test_options = options; }
		void SetCullMode(CullMode cull_mode) { m_cull_mode = cull_mode; }
		void SetBlendOptions(BlendOptions const & options) { m_blend_options = options; }

		void SetPerFrameConstantsCallback(PerFrameConstantsCallback callback) { m_per_frame_constants_callback = callback; }
		void SetPerObjectConstantsCallback(PerObjectConstantsCallback callback) { m_per_object_constants_callback = callback; }

		std::expected<Pipeline, GraphicsError> CreatePipeline() const;

	private:
		RenderContext const & m_render_context;

		unsigned int m_vert_shader_id = 0;
		unsigned int m_frag_shader_id = 0;

		size_t m_vs_object_uniform_size = 0;
		size_t m_fs_object_uniform_size = 0;
		std::vector<size_t> m_vs_uniform_sizes;
		std::vector<size_t> m_fs_uniform_sizes;
		bool m_has_texture = false;

		DepthTestOptions m_depth_test_options;
		BlendOptions m_blend_options;

		// PipelineBuilder does not have a default state for the cull mode, a null optional here means "not set yet".
		// PipelineBuilder requires the cull mode to be set explicitly before calling CreatePipeline().
		std::optional<CullMode> m_cull_mode;

		PerFrameConstantsCallback m_per_frame_constants_callback;
		PerObjectConstantsCallback m_per_object_constants_callback;
	};

	template <VertexWithLayout VertexT>
	void PipelineBuilder::SetVertexType()
	{
		// In OpenGL, vertex attributes are set when binding the VAO for each mesh.
	}

	template <typename InstanceDataT>
	void PipelineBuilder::SetInstanceType()
	{
		// In OpenGL, instance attributes are set when binding the VAO for each mesh.
	}

	template <typename ObjectDataVS /*= std::nullopt_t*/, typename ObjectDataFS /*= std::nullopt_t*/>
	void PipelineBuilder::SetObjectDataTypes()
	{
		static_assert(!std::same_as<ObjectDataVS, std::nullopt_t> || !std::same_as<ObjectDataFS, std::nullopt_t>,
			"At least one object data type must be provided");

		if constexpr (!std::same_as<ObjectDataVS, std::nullopt_t>)
			m_vs_object_uniform_size = sizeof(ObjectDataVS);

		if constexpr (!std::same_as<ObjectDataFS, std::nullopt_t>)
			m_fs_object_uniform_size = sizeof(ObjectDataFS);
	}

	template <typename... UniformTypes>
	void PipelineBuilder::SetVSUniformTypes()
	{
		m_vs_uniform_sizes = {
			sizeof(UniformTypes)...
		};
	}

	template <typename... UniformTypes>
	void PipelineBuilder::SetFSUniformTypes()
	{
		m_fs_uniform_sizes = {
			sizeof(UniformTypes)...
		};
	}
} // namespace Dreamhearth
