// PipelineBuilder.ixx

module;

#include <filesystem>

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

	void SetDepthTestOptions(DepthTestOptions const & options) { m_depth_test_options = options; }

	void SetPerFrameConstantsCallback(PerFrameConstantsCallback callback) { m_per_frame_constants_callback = callback; }
	void SetPerObjectConstantsCallback(PerObjectConstantsCallback callback) { m_per_object_constants_callback = callback; }

	std::optional<GraphicsPipeline> CreatePipeline() const;

private:
	unsigned int m_vert_shader_id{ 0 };
	unsigned int m_frag_shader_id{ 0 };

	DepthTestOptions m_depth_test_options;

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};
