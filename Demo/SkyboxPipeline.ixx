// SkyboxPipeline.ixx

module;

#include <expected>
#include <filesystem>

export module SkyboxPipeline;

import Dreamhearth;

import AssetPool;
import Camera;
import RenderObject;
import Vertex;

using namespace Dreamhearth;

export class SkyboxPipeline
{
public:
	using VertexT = PositionVertex;

	static std::expected<Pipeline, GraphicsError> CreatePipeline(
		RenderContext const & render_context,
		std::filesystem::path const & shaders_path,
		Camera const & camera);

	SkyboxPipeline() = default;
	explicit SkyboxPipeline(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
};

std::expected<Pipeline, GraphicsError> SkyboxPipeline::CreatePipeline(
	RenderContext const & render_context,
	std::filesystem::path const & shaders_path,
	Camera const & camera)
{
	PipelineBuilder builder{ render_context };

	std::expected<void, GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "skybox.vert",
		shaders_path / "skybox.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetHasTexture(true);
	builder.SetDepthTestOptions(DepthTestOptions{
		.enable_depth_test = true,
		.enable_depth_write = false,
		.depth_compare_op = DepthCompareOp::EQUAL
		});
	builder.SetCullMode(CullMode::BACK);

	builder.SetPerFrameConstantsCallback(
		[&camera](Pipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
		});

	return builder.CreatePipeline();
}
