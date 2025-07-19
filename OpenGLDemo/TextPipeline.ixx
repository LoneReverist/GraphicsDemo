// TextPipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/glm.hpp>

export module TextPipeline;

import AssetId;
import FontAtlas;
import GraphicsPipeline;
import PipelineBuilder;
import Renderer;
import Vertex;

namespace TextPipeline
{
	export using VertexT = Texture2dVertex;

	std::optional<GraphicsPipeline> create_text_pipeline(
		FontAtlas const & font_atlas,
		std::filesystem::path const & shaders_path)
	{
		PipelineBuilder builder;
		builder.LoadShaders(
			shaders_path / "msdf_text.vert",
			shaders_path / "msdf_text.frag");

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

	export auto Create(
		Renderer & renderer,
		FontAtlas const & font_atlas,
		std::filesystem::path const & shaders_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<GraphicsPipeline> pipeline = create_text_pipeline(font_atlas, shaders_path);
		if (!pipeline.has_value())
		{
			std::cout << "Failed to create TextPipeline" << std::endl;
			return id;
		}

		id.m_index = renderer.AddPipeline(std::move(pipeline.value()));
		return id;
	}
}
