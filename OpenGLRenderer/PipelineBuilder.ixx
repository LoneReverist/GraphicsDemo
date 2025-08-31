// PipelineBuilder.ixx

module;

#include <filesystem>
#include <optional>
#include <vector>

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

	template <IsVertex VertexT>
	void SetVertexType();

	template <typename ObjectDataVS = std::nullopt_t, typename ObjectDataFS = std::nullopt_t>
	void SetObjectDataTypes();

	template <typename... UniformTypes>
	void SetVSUniformTypes();

	template <typename... UniformTypes>
	void SetFSUniformTypes();

	void SetTexture(Texture const & texture) { m_texture = &texture; }
	void SetDepthTestOptions(DepthTestOptions const & options) { m_depth_test_options = options; }
	void SetCullMode(CullMode cull_mode) { m_cull_mode = cull_mode; }
	void SetBlendOptions(BlendOptions const & options) { m_blend_options = options; }

	void SetPerFrameConstantsCallback(PerFrameConstantsCallback callback) { m_per_frame_constants_callback = callback; }
	void SetPerObjectConstantsCallback(PerObjectConstantsCallback callback) { m_per_object_constants_callback = callback; }

	std::optional<GraphicsPipeline> CreatePipeline() const;

private:
	unsigned int m_vert_shader_id = 0;
	unsigned int m_frag_shader_id = 0;

	size_t m_vs_object_uniform_size = 0;
	size_t m_fs_object_uniform_size = 0;
	std::vector<size_t> m_vs_uniform_sizes;
	std::vector<size_t> m_fs_uniform_sizes;
	Texture const * m_texture = nullptr;

	DepthTestOptions m_depth_test_options;
	BlendOptions m_blend_options;
	CullMode m_cull_mode = CullMode::NONE; // Default to none to ensure objects always appear on screen when testing new pipelines.

	PerFrameConstantsCallback m_per_frame_constants_callback;
	PerObjectConstantsCallback m_per_object_constants_callback;
};

template <IsVertex VertexT>
void PipelineBuilder::SetVertexType()
{
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
