// PipelineBuilder.ixx

module;

#include <filesystem>
#include <optional>

export module PipelineBuilder;

import GraphicsApi;
import GraphicsPipeline;
import RenderObject;
import Vertex;

export class PipelineBuilder
{
public:
	using PerFrameConstantsCallback = GraphicsPipeline::PerFrameConstantsCallback;
	using PerObjectConstantsCallback = GraphicsPipeline::PerObjectConstantsCallback;

	PipelineBuilder() = default;
	~PipelineBuilder();

	void LoadShaders(std::filesystem::path const & vs_path, std::filesystem::path const & fs_path);

	template <typename... UniformTypes>
	void SetVSUniformTypes();

	template <typename... UniformTypes>
	void SetFSUniformTypes();

	void SetDepthTestOptions(DepthTestOptions const & options) { m_depth_test_options = options; }
	void SetCullMode(CullMode cull_mode) { m_cull_mode = cull_mode; }

	void SetPerFrameConstantsCallback(PerFrameConstantsCallback callback) { m_per_frame_constants_callback = callback; }
	void SetPerObjectConstantsCallback(PerObjectConstantsCallback callback) { m_per_object_constants_callback = callback; }

	std::optional<GraphicsPipeline> CreatePipeline() const;

private:
	unsigned int m_vert_shader_id = 0;
	unsigned int m_frag_shader_id = 0;

	std::vector<size_t> m_vs_uniform_sizes;
	std::vector<size_t> m_fs_uniform_sizes;

	DepthTestOptions m_depth_test_options;
	CullMode m_cull_mode = CullMode::NONE; // Default to none to ensure objects always appear on screen when testing new pipelines.

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

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
