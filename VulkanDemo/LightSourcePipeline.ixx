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
import RenderObject;
import Vertex;

export class LightSourcePipeline
{
public:
	using VertexT = NormalVertex;
	using AssetIdT = AssetId<VertexT>;

	struct ObjectData
	{
		glm::mat4 m_model{ 1.0 };
		glm::vec3 m_color{ 0.0 };
	};

	static std::optional<GraphicsPipeline> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera);

	LightSourcePipeline() = default;
	LightSourcePipeline(AssetIdT asset_id) : m_asset_id(asset_id) {}

	AssetIdT GetAssetId() const { return m_asset_id; }

private:
	AssetIdT m_asset_id;
};

std::optional<GraphicsPipeline> LightSourcePipeline::CreateGraphicsPipeline(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & shaders_path,
	Camera const & camera)
{
	struct ObjectDataVS
	{
		alignas(16) glm::mat4 m_model;
	};
	struct ObjectDataFS
	{
		alignas(16) glm::vec3 m_color;
	};

	PipelineBuilder builder{ graphics_api };
	builder.LoadShaders(
		shaders_path / "light_source.vert",
		shaders_path / "light_source.frag");
	builder.SetVertexType<VertexT>();
	builder.SetObjectDataTypes<ObjectDataVS, ObjectDataFS>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetFSUniformTypes<CameraPosUniform>();
	builder.SetCullMode(CullMode::BACK);

	builder.SetPerFrameConstantsCallback(
		[&camera](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
			pipeline.SetUniform(1 /*binding*/, camera.GetPosUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](GraphicsPipeline const & pipeline, RenderObject const & obj)
		{
			// For optimal performance, we assume that the object data is of the correct type.
			// Use compile-time checks when creating render objects to ensure the data is compatible with the pipeline.
			auto const * data = static_cast<ObjectData const *>(obj.GetObjectData());
			if (!data)
			{
				std::cout << "ObjectData is null for LightSourcePipeline" << std::endl;
				return;
			}

			pipeline.SetObjectData(
				ObjectDataVS{
					.m_model = data->m_model
				},
				ObjectDataFS{
					.m_color = data->m_color,
				});
		});

	return builder.CreatePipeline();
}
