// ColorPipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/mat4x4.hpp>

export module ColorPipeline;

import AssetId;
import Camera;
import GraphicsApi;
import GraphicsPipeline;
import LightsManager;
import PipelineBuilder;
import Vertex;

export class ColorPipeline
{
public:
	using VertexT = ColorVertex;

	struct ObjectData
	{
		glm::mat4 m_model{ 1.0 };
	};

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera,
		LightsManager const & lights);

	ColorPipeline() = default;
	ColorPipeline(AssetId<VertexT> asset_id) : m_asset_id(asset_id) {}

	AssetId<VertexT> GetAssetId() const { return m_asset_id; }

private:
	AssetId<VertexT> m_asset_id;
};

std::optional<GraphicsPipeline> ColorPipeline::CreateGraphicsPipeline(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & shaders_path,
	Camera const & camera,
	LightsManager const & lights)
{
	struct VSPushConstant
	{
		alignas(16) glm::mat4 m_model;
	};

	PipelineBuilder builder{ graphics_api };
	builder.LoadShaders(
		shaders_path / "color.vert.spv",
		shaders_path / "color.frag.spv");
	builder.SetVertexType<VertexT>();
	builder.SetPushConstantTypes<VSPushConstant, std::nullopt_t>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetFSUniformTypes<LightsUniform>();

	builder.SetPerFrameConstantsCallback(
		[&camera, &lights](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
			pipeline.SetUniform(1 /*binding*/, lights.GetLightsUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			// For optimal performance, we assume that the object data is of the correct type.
			// Use compile-time checks when creating render objects to ensure the data is compatible with the pipeline.
			auto const * data = static_cast<ObjectData const *>(obj.GetPipelineData());
			if (!data)
			{
				std::cout << "ObjectData is null for ColorPipeline" << std::endl;
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
