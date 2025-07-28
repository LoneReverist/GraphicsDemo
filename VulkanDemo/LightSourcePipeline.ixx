// LightSourcePipeline.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/mat4x4.hpp>

export module LightSourcePipeline;

import AssetId;
import Camera;
import GraphicsApi;
import GraphicsPipeline;
import PipelineBuilder;
import Vertex;

export class LightSourcePipeline
{
public:
	using VertexT = NormalVertex;

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera);

	LightSourcePipeline() = default;
	LightSourcePipeline(AssetId<VertexT> asset_id) : m_asset_id(asset_id) {}

	AssetId<VertexT> GetAssetId() const { return m_asset_id; }

private:
	AssetId<VertexT> m_asset_id;
};

std::optional<GraphicsPipeline> LightSourcePipeline::CreateGraphicsPipeline(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & shaders_path,
	Camera const & camera)
{
	struct VSPushConstant
	{
		alignas(16) glm::mat4 m_model;
	};
	struct FSPushConstant
	{
		alignas(16) glm::vec3 m_color;
	};

	PipelineBuilder builder{ graphics_api };
	builder.LoadShaders(
		shaders_path / "light_source.vert.spv",
		shaders_path / "light_source.frag.spv");
	builder.SetVertexType<VertexT>();
	builder.SetPushConstantTypes<VSPushConstant, FSPushConstant>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetFSUniformTypes<CameraPosUniform>();

	builder.SetPerFrameConstantsCallback(
		[&camera](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
			pipeline.SetUniform(1 /*binding*/, camera.GetPosUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			pipeline.SetPushConstants(
				VSPushConstant{
					.m_model = obj.GetModelTransform()
				},
				FSPushConstant{
					.m_color = obj.GetColor(),
				});
		});

	return builder.CreatePipeline();
}
