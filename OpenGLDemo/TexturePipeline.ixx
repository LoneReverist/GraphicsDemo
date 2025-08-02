// TexturePipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/mat4x4.hpp>

export module TexturePipeline;

import AssetId;
import Camera;
import GraphicsApi;
import GraphicsPipeline;
import LightsManager;
import PipelineBuilder;
import Texture;
import Vertex;

export class TexturePipeline
{
public:
	using VertexT = TextureVertex;
	using AssetIdT = AssetId<VertexT>;

	struct ObjectData
	{
		glm::mat4 m_model{ 1.0 };
	};

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera,
		LightsManager const & lights,
		Texture const & texture);

	TexturePipeline() = default;
	TexturePipeline(AssetIdT asset_id) : m_asset_id(asset_id) {}

	AssetIdT GetAssetId() const { return m_asset_id; }

private:
	AssetIdT m_asset_id;
};

std::optional<GraphicsPipeline> TexturePipeline::CreateGraphicsPipeline(
	GraphicsApi const & /*graphics_api*/,
	std::filesystem::path const & shaders_path,
	Camera const & camera,
	LightsManager const & lights,
	Texture const & texture)
{
	PipelineBuilder builder;
	builder.LoadShaders(
		shaders_path / "texture.vert",
		shaders_path / "texture.frag");
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetFSUniformTypes<LightsUniform>();
	builder.SetCullMode(CullMode::BACK);

	builder.SetPerFrameConstantsCallback(
		[&camera, &lights](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
			pipeline.SetUniform(1 /*binding*/, lights.GetLightsUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[&texture](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			// For optimal performance, we assume that the object data is of the correct type.
			// Use compile-time checks when creating render objects to ensure the data is compatible with the pipeline.
			auto const * data = static_cast<ObjectData const *>(obj.GetObjectData());
			if (!data)
			{
				std::cout << "ObjectData is null for TexturePipeline" << std::endl;
				return;
			}

			pipeline.SetUniform("model_transform", data->m_model);

			texture.Bind();
		});

	return builder.CreatePipeline();
}
