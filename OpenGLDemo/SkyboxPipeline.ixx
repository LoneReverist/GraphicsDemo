// SkyboxPipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

export module SkyboxPipeline;

import AssetId;
import Camera;
import GraphicsApi;
import GraphicsPipeline;
import PipelineBuilder;
import RenderObject;
import Texture;
import Vertex;

export class SkyboxPipeline
{
public:
	using VertexT = PositionVertex;
	using AssetIdT = AssetId<VertexT>;

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera,
		Texture const & skybox);

	SkyboxPipeline() = default;
	SkyboxPipeline(AssetIdT asset_id) : m_asset_id(asset_id) {}

	AssetIdT GetAssetId() const { return m_asset_id; }

private:
	AssetIdT m_asset_id;
};

std::optional<GraphicsPipeline> SkyboxPipeline::CreateGraphicsPipeline(
	GraphicsApi const & /*graphics_api*/,
	std::filesystem::path const & shaders_path,
	Camera const & camera,
	Texture const & skybox)
{
	PipelineBuilder builder;
	builder.LoadShaders(
		shaders_path / "skybox.vert",
		shaders_path / "skybox.frag");
	builder.SetVSUniformTypes<ViewProjUniform>();
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
	builder.SetPerObjectConstantsCallback(
		[&skybox](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			skybox.Bind();
		});

	return builder.CreatePipeline();
}
