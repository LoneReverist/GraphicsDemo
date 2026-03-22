// SkyboxPipeline.ixx

module;

#include <expected>
#include <filesystem>

export module SkyboxPipeline;

import AssetPool;
import Camera;
import GraphicsApi;
import GraphicsError;
import GraphicsPipeline;
import PipelineBuilder;
import RenderObject;
import Texture;
import Vertex;

export class SkyboxPipeline
{
public:
	using VertexT = PositionVertex;

	static std::expected<GraphicsPipeline, GraphicsError> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera,
		AssetPool<Texture> const & texture_pool,
		AssetId texture_id);

	SkyboxPipeline() = default;
	explicit SkyboxPipeline(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
};

std::expected<GraphicsPipeline, GraphicsError> SkyboxPipeline::CreateGraphicsPipeline(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & shaders_path,
	Camera const & camera,
	AssetPool<Texture> const & texture_pool,
	AssetId texture_id)
{
	Texture const * texture = texture_pool.Get(texture_id);
	if (!texture)
		return std::unexpected{ GraphicsError{ "SkyboxPipeline::CreateGraphicsPipeline: invalid texture" } };

	PipelineBuilder builder{ graphics_api };

	std::expected<void, GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "skybox.vert",
		shaders_path / "skybox.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetTexture(*texture);
	builder.SetDepthTestOptions(DepthTestOptions{
		.m_enable_depth_test = true,
		.m_enable_depth_write = false,
		.m_depth_compare_op = DepthCompareOp::EQUAL
		});
	builder.SetCullMode(CullMode::BACK);

	builder.SetPerFrameConstantsCallback(
		[&camera](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
		});

	return builder.CreatePipeline();
}
