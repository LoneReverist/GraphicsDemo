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
import RenderObject;
import Vertex;

export class TextPipeline
{
public:
	using VertexT = Texture2dVertex;
	using AssetIdT = AssetId<VertexT>;

	struct ObjectData
	{
		float m_screen_px_range = 1.0f;
		glm::vec4 m_bg_color = glm::vec4(0.0f);
		glm::vec4 m_text_color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	};

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		FontAtlas const & font_atlas);

	TextPipeline() = default;
	TextPipeline(AssetIdT asset_id) : m_asset_id(asset_id) {}

	AssetIdT GetAssetId() const { return m_asset_id; }

private:
	AssetIdT m_asset_id;
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
	builder.SetBlendOptions(BlendOptions{
		.m_enable_blend = true,
		.m_src_factor = BlendFactor::SRC_ALPHA,
		.m_dst_factor = BlendFactor::ONE_MINUS_SRC_ALPHA
		});
	builder.SetCullMode(CullMode::BACK);

	builder.SetPerObjectConstantsCallback(
		[&font_atlas](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			// For optimal performance, we assume that the object data is of the correct type.
			// Use compile-time checks when creating render objects to ensure the data is compatible with the pipeline.
			auto const * data = static_cast<ObjectData const *>(obj.GetObjectData());
			if (!data)
			{
				std::cout << "TextObjectData is null for TextPipeline" << std::endl;
				return;
			}

			pipeline.SetUniform("screen_px_range", data->m_screen_px_range);

			pipeline.SetUniform("bg_color", data->m_bg_color);
			pipeline.SetUniform("text_color", data->m_text_color);

			font_atlas.GetTexture().Bind();
		});

	return builder.CreatePipeline();
}
