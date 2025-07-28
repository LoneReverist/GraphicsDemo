// ReflectionPipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/mat4x4.hpp>

export module ReflectionPipeline;

import AssetId;
import Camera;
import GraphicsApi;
import GraphicsPipeline;
import LightsManager;
import PipelineBuilder;
import Texture;
import Vertex;

export class ReflectionPipeline
{
public:
	using VertexT = NormalVertex;

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera,
		LightsManager const & lights,
		Texture const & texture);

	ReflectionPipeline() = default;
	ReflectionPipeline(AssetId<VertexT> asset_id) : m_asset_id(asset_id) {}

	AssetId<VertexT> GetAssetId() const { return m_asset_id; }

private:
	AssetId<VertexT> m_asset_id;
};

std::optional<GraphicsPipeline> ReflectionPipeline::CreateGraphicsPipeline(
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
		shaders_path / "reflection.vert.spv",
		shaders_path / "reflection.frag.spv");
	builder.SetVertexType<VertexT>();
	builder.SetPushConstantTypes<VSPushConstant, std::nullopt_t>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetFSUniformTypes<LightsUniform, CameraPosUniform>();
	builder.SetTexture(texture);

	builder.SetPerFrameConstantsCallback(
		[&camera, &lights](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
			pipeline.SetUniform(1 /*binding*/, lights.GetLightsUniform());
			pipeline.SetUniform(2 /*binding*/, camera.GetPosUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			pipeline.SetPushConstants(
				VSPushConstant{
					.m_model = obj.GetModelTransform()
				},
				std::nullopt);
		});

	return builder.CreatePipeline();
}
