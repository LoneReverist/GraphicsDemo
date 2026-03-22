// ReflectionPipeline.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <optional>

#include <glm/mat4x4.hpp>

export module ReflectionPipeline;

import AssetPool;
import Camera;
import GraphicsApi;
import GraphicsError;
import GraphicsPipeline;
import LightsManager;
import PipelineBuilder;
import RenderObject;
import Texture;
import Vertex;

export class ReflectionPipeline
{
public:
	using VertexT = NormalVertex;

	struct ObjectData
	{
		glm::mat4 model{ 1.0 };
	};

	static std::expected<GraphicsPipeline, GraphicsError> CreateGraphicsPipeline(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & shaders_path,
		Camera const & camera,
		LightsManager const & lights,
		AssetPool<Texture> const & texture_pool,
		AssetId texture_id);

	ReflectionPipeline() = default;
	explicit ReflectionPipeline(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
};

std::expected<GraphicsPipeline, GraphicsError> ReflectionPipeline::CreateGraphicsPipeline(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & shaders_path,
	Camera const & camera,
	LightsManager const & lights,
	AssetPool<Texture> const & texture_pool,
	AssetId texture_id)
{
	struct ObjectDataVS
	{
		alignas(16) glm::mat4 model;
	};

	Texture const * texture = texture_pool.Get(texture_id);
	if (!texture)
		return std::unexpected{ GraphicsError{ "ReflectionPipeline::CreateGraphicsPipeline: invalid texture" } };

	PipelineBuilder builder{ graphics_api };

	std::expected<void, GraphicsError> load_shaders_result = builder.LoadShaders(
		shaders_path / "reflection.vert",
		shaders_path / "reflection.frag");
	if (!load_shaders_result.has_value())
		return std::unexpected{ load_shaders_result.error() };

	builder.SetVertexType<VertexT>();
	builder.SetObjectDataTypes<ObjectDataVS, std::nullopt_t>();
	builder.SetVSUniformTypes<ViewProjUniform>();
	builder.SetFSUniformTypes<LightsUniform, CameraPosUniform>();
	builder.SetTexture(*texture);
	builder.SetCullMode(CullMode::BACK);

	builder.SetPerFrameConstantsCallback(
		[&camera, &lights](GraphicsPipeline const & pipeline)
		{
			pipeline.SetUniform(0 /*binding*/, camera.GetViewProjUniform());
			pipeline.SetUniform(1 /*binding*/, lights.GetLightsUniform());
			pipeline.SetUniform(2 /*binding*/, camera.GetPosUniform());
		});
	builder.SetPerObjectConstantsCallback(
		[](GraphicsPipeline const & pipeline, void const * object_data)
		{
			if (!object_data)
			{
				std::cout << "ObjectData is null for ReflectionPipeline" << std::endl;
				return;
			}

			// For optimal performance, we assume that the object data is of the correct type.
			// Use compile-time checks when creating render objects to ensure the data is compatible with the pipeline.
			auto const * data = static_cast<ObjectData const *>(object_data);

			pipeline.SetObjectData(
				ObjectDataVS{
					.model = data->model
				},
				std::nullopt);
		});

	return builder.CreatePipeline();
}
