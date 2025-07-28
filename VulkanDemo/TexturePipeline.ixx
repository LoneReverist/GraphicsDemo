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
	TexturePipeline(AssetId<VertexT> asset_id) : m_asset_id(asset_id) {}

	AssetId<VertexT> GetAssetId() const { return m_asset_id; }

private:
	AssetId<VertexT> m_asset_id;
};

std::optional<GraphicsPipeline> TexturePipeline::CreateGraphicsPipeline(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & shaders_path,
	Camera const & camera,
	LightsManager const & lights,
	Texture const & texture)
{
	struct VSPushConstant
	{
		alignas(16) glm::mat4 m_model;
	};

	PipelineBuilder builder{ graphics_api };
	builder.LoadShaders(
		shaders_path / "texture.vert.spv",
		shaders_path / "texture.frag.spv");
	builder.SetVertexType<VertexT>();
	builder.SetPushConstantTypes<VSPushConstant, std::nullopt_t>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetFSUniformTypes<LightsUniform>();
	builder.SetTexture(texture);

	builder.SetPerFrameConstantsCallback(
		[&camera, &lights](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
			pipeline.SetUniform(1 /*binding*/, lights.GetLightsUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			auto const * data = static_cast<ObjectData const *>(obj.GetPipelineData());
			if (!data)
			{
				std::cout << "ObjectData is null for TexturePipeline" << std::endl;
				return;
			}

			pipeline.SetPushConstants(
				VSPushConstant{
					.m_model = data->m_model
				},
				std::nullopt);
		});

	return builder.CreatePipeline();
}
