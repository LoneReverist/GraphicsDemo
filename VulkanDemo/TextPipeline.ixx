// TextPipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/glm.hpp>

export module TextPipeline;

import AssetId;
import FontAtlas;
import GraphicsApi;
import GraphicsPipeline;
import PipelineBuilder;
import Renderer;
import Vertex;

namespace TextPipeline
{
	export using VertexT = Texture2dVertex;

	std::optional<GraphicsPipeline> create_text_pipeline(
		GraphicsApi const & graphics_api,
		FontAtlas const & font_atlas,
		std::filesystem::path const & shaders_path)
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
				float scale = 4.0f; // TODO: font size * dpi scaling?
				float screen_px_range = scale * font_atlas.GetPxRange();

				pipeline.SetPushConstants(
					std::nullopt,
					FSPushConstant{
						.screen_px_range = screen_px_range,
						.bg_color = glm::vec4(0.0, 0.0, 0.0, 0.0),
						.text_color = glm::vec4(obj.GetColor(), 1.0)
					});
			});

		return builder.CreatePipeline();
	}

	export auto Create(
		GraphicsApi const & graphics_api,
		Renderer & renderer,
		FontAtlas const & font_atlas,
		std::filesystem::path const & shaders_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<GraphicsPipeline> pipeline = create_text_pipeline(graphics_api, font_atlas, shaders_path);
		if (!pipeline.has_value())
		{
			std::cout << "Failed to create TextPipeline" << std::endl;
			return id;
		}

		id.m_index = renderer.AddPipeline(std::move(pipeline.value()));
		return id;
	}
}
