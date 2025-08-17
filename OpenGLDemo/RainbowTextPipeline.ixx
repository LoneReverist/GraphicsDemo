// RainbowTextPipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/vec4.hpp>

export module RainbowTextPipeline;

import AssetId;
import FontAtlas;
import GraphicsApi;
import GraphicsPipeline;
import PipelineBuilder;
import RenderObject;
import Vertex;

export class RainbowTextPipeline
{
public:
	using VertexT = Texture2dVertex;
	using AssetIdT = AssetId<VertexT>;

	struct ObjectData
	{
		glm::vec4 m_bg_color = glm::vec4(0.0f);
		float m_screen_px_range = 1.0f;
		float m_time = 0.0f;
		float m_rainbow_width = 200.0f; // in pixels, adjust for dpi

		// Adjust for slant angle (0.0 = vertical, 1.0 = 45 degrees forward, -1.0 = 45 degrees backward)
		float m_slant_factor = -1.0f;
	};

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		FontAtlas const & font_atlas);

	RainbowTextPipeline() = default;
	RainbowTextPipeline(AssetIdT asset_id) : m_asset_id(asset_id) {}

	AssetIdT GetAssetId() const { return m_asset_id; }

private:
	AssetIdT m_asset_id;
};

std::optional<GraphicsPipeline> RainbowTextPipeline::CreateGraphicsPipeline(
	GraphicsApi const & /*graphics_api*/,
	std::filesystem::path const & shaders_path,
	FontAtlas const & font_atlas)
{
	PipelineBuilder builder;
	builder.LoadShaders(
		shaders_path / "msdf_text.vert",
		shaders_path / "rainbow_text.frag");
	builder.SetDepthTestOptions(DepthTestOptions{
		.m_enable_depth_test = false,
		.m_enable_depth_write = false,
		.m_depth_compare_op = DepthCompareOp::ALWAYS
		});
	builder.SetBlendOptions(BlendOptions{
		.m_enable_blend = true,
		.m_src_factor = BlendFactor::SRC_ALPHA,
		.m_dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	builder.SetCullMode(CullMode::BACK);

	builder.SetPerObjectConstantsCallback(
		[&font_atlas](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			auto const * data = static_cast<ObjectData const *>(obj.GetObjectData());
			if (!data)
			{
				std::cout << "TextObjectData is null for RainbowTextPipeline" << std::endl;
				return;
			}

			pipeline.SetUniform("bg_color", data->m_bg_color);
			pipeline.SetUniform("screen_px_range", data->m_screen_px_range);
			pipeline.SetUniform("time", data->m_time);
			pipeline.SetUniform("rainbow_width", data->m_rainbow_width);
			pipeline.SetUniform("slant_factor", data->m_slant_factor);

			font_atlas.GetTexture().Bind();
		});

	return builder.CreatePipeline();
}
