// TextPipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/vec4.hpp>

export module TextPipeline;

import AssetId;
import FontAtlas;
import GraphicsApi;
import GraphicsPipeline;
import PipelineBuilder;
import Vertex;

export class TextPipeline
{
public:
	using VertexT = Texture2dVertex;

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		FontAtlas const & font_atlas);

	TextPipeline() = default;
	TextPipeline(AssetId<VertexT> asset_id) : m_asset_id(asset_id) {}

	AssetId<VertexT> GetAssetId() const { return m_asset_id; }

private:
	AssetId<VertexT> m_asset_id;
};

std::optional<GraphicsPipeline> TextPipeline::CreateGraphicsPipeline(
	GraphicsApi const & /*graphics_api*/,
	std::filesystem::path const & shaders_path,
	FontAtlas const & font_atlas)
{
	PipelineBuilder builder;
	builder.LoadShaders(
		shaders_path / "msdf_text.vert",
		shaders_path / "msdf_text.frag");
	builder.SetDepthTestOptions(DepthTestOptions{
		.m_enable_depth_test = false,
		.m_enable_depth_write = false,
		.m_depth_compare_op = DepthCompareOp::ALWAYS
		});

	builder.SetPerObjectConstantsCallback(
		[&font_atlas](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			float scale = 4.0f; // TODO: font size * dpi scaling?
			float screen_px_range = scale * font_atlas.GetPxRange();
			pipeline.SetUniform("screen_px_range", screen_px_range);

			pipeline.SetUniform("bg_color", glm::vec4(0.0, 0.0, 0.0, 0.0));
			pipeline.SetUniform("text_color", glm::vec4(obj.GetColor(), 1.0));

			font_atlas.GetTexture().Bind();
		});

	return builder.CreatePipeline();
}
