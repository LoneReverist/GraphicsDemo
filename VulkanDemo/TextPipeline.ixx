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

	struct ObjectData
	{
		float screen_px_range = 1.0f;
		glm::vec4 bg_color = glm::vec4(0.0f);
		glm::vec4 text_color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	};

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
	GraphicsApi const & graphics_api,
	std::filesystem::path const & shaders_path,
	FontAtlas const & font_atlas)
{
	struct FSPushConstant
	{
		alignas(4) float screen_px_range;
		alignas(16) glm::vec4 bg_color;
		alignas(16) glm::vec4 text_color;
	};

	PipelineBuilder builder{ graphics_api };
	builder.LoadShaders(
		shaders_path / "msdf_text.vert.spv",
		shaders_path / "msdf_text.frag.spv");
	builder.SetVertexType<VertexT>();
	builder.SetPushConstantTypes<std::nullopt_t, FSPushConstant>();
	builder.SetTexture(font_atlas.GetTexture());
	builder.SetDepthTestOptions(DepthTestOptions{
		.m_enable_depth_test = false,
		.m_enable_depth_write = false,
		.m_depth_compare_op = DepthCompareOp::ALWAYS
		});

		builder.SetPerObjectConstantsCallback(
			[&font_atlas](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				// For optimal performance, we assume that the object data is of the correct type.
				// Use compile-time checks when creating render objects to ensure the data is compatible with the pipeline.
				auto const * data = static_cast<ObjectData const *>(obj.GetPipelineData());
				if (!data)
				{
					std::cout << "TextObjectData is null for TextPipeline" << std::endl;
					return;
				}

				pipeline.SetPushConstants(
					std::nullopt,
					FSPushConstant{
						.screen_px_range = data->screen_px_range,
						.bg_color = data->bg_color,
						.text_color = data->text_color
					});
			});

	return builder.CreatePipeline();
}
