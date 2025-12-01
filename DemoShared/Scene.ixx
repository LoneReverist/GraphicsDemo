// Scene.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

export module Scene;

import Camera;
import ColorPipeline;
import FontAtlas;
import GraphicsApi;
import GraphicsError;
import Input;
import LightsManager;
import LightSourcePipeline;
import MeshAsset;
import RainbowTextPipeline;
import ReflectionPipeline;
import Renderer;
import RenderObject;
import SkyboxPipeline;
import TextMesh;
import TextPipeline;
import Texture;
import TexturePipeline;
import Vertex;

export class Scene
{
public:
	explicit Scene(GraphicsApi const & graphics_api, std::string const & title);

	void OnViewportResized(int width, int height);

	void Update(double delta_time, Input const & input);
	void Render() const;

private:
	template <IsVertex VertexT, typename... Args>
	MeshAsset<VertexT> create_mesh(Args &&... args);

	template <typename PipelineT, typename... Args>
	PipelineT create_pipeline(Args &&... args);

	template <typename Mesh, typename Pipeline, typename ObjectData = std::nullopt_t>
		requires MeshIsCompatibleWithPipeline<Mesh, Pipeline>
			&& (ObjectDataIsCompatibleWithPipeline<ObjectData, Pipeline>
			|| std::same_as<ObjectData, std::nullopt_t>)
	void create_render_object(
		std::string const & name,
		Mesh const & mesh,
		Pipeline const & pipeline,
		ObjectData const & object_data = std::nullopt);

	MeshAsset<PositionVertex> create_skybox_mesh();
	MeshAsset<TextureVertex> create_ground_mesh();
	std::vector<MeshAsset<ColorVertex>> create_tree_with_material_meshes();
	std::unique_ptr<TextMesh> create_text_mesh(
		std::string const & text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		int viewport_width,
		int viewport_height);

private:
	GraphicsApi const & m_graphics_api;
	std::filesystem::path const m_resources_path;
	std::string const m_title;

	Renderer m_renderer;
	Camera m_camera;

	std::unique_ptr<Texture> m_ground_tex;
	std::unique_ptr<Texture> m_skybox_tex;
	std::unique_ptr<FontAtlas> m_arial_font;

	std::unique_ptr<TextMesh> m_fps_mesh;
	std::unique_ptr<TextMesh> m_title_mesh;

	std::vector<std::shared_ptr<RenderObject>> m_render_objs;

	ReflectionPipeline::ObjectData m_sword0;
	ReflectionPipeline::ObjectData m_sword1;
	LightSourcePipeline::ObjectData m_red_gem;
	LightSourcePipeline::ObjectData m_green_gem;
	LightSourcePipeline::ObjectData m_blue_gem;
	TexturePipeline::ObjectData m_ground;
	TextPipeline::ObjectData m_fps_label;
	RainbowTextPipeline::ObjectData m_title_label;
	LightSourcePipeline::ObjectData m_tree;
	ColorPipeline::ObjectData m_tree_with_material;

	LightsManager m_lights;

	float m_timer = 0.0f;
	float m_frame_timer = 0.0f;
	int m_frame_count = 0;

	float m_dpi_scale = 1.0f;
};

template<IsVertex VertexT, typename... Args>
MeshAsset<VertexT> Scene::create_mesh(Args &&... args)
{
	std::expected<int, GraphicsError> mesh_id
		= MeshAsset<VertexT>::Create(m_graphics_api, std::forward<Args>(args)...)
		.and_then([&renderer = m_renderer](Mesh mesh)-> std::expected<int, GraphicsError>
			{
				return renderer.AddMesh(std::move(mesh));
			});

	if (!mesh_id.has_value())
	{
		std::cout << "Failed to create MeshAsset<" << typeid(VertexT).name()
			<< "> Error: " << mesh_id.error().GetMessage() << std::endl;
		return MeshAsset<VertexT>{};
	}

	return MeshAsset<VertexT>{ AssetId{ mesh_id.value() } };
}

template <typename PipelineT, typename... Args>
PipelineT Scene::create_pipeline(Args &&... args)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";

	std::expected<int, GraphicsError> pipeline_id
		= PipelineT::CreateGraphicsPipeline(m_graphics_api, shaders_path, std::forward<Args>(args)...)
		.and_then([&renderer = m_renderer](GraphicsPipeline pipeline)-> std::expected<int, GraphicsError>
			{
				return renderer.AddPipeline(std::move(pipeline));
			});

	if (!pipeline_id.has_value())
	{
		std::cout << "Failed to create " << typeid(PipelineT).name()
			<< " Error: " << pipeline_id.error().GetMessage() << std::endl;
		return PipelineT{};
	}

	return PipelineT{ AssetId{ pipeline_id.value() } };
}

template <typename Mesh, typename Pipeline>
concept MeshIsCompatibleWithPipeline = std::same_as<typename Mesh::VertexT, typename Pipeline::VertexT>;

template <typename ObjectData, typename Pipeline>
concept ObjectDataIsCompatibleWithPipeline = std::same_as<ObjectData, typename Pipeline::ObjectData>;

template <typename Mesh, typename Pipeline, typename ObjectData /*= std::nullopt_t*/>
	requires MeshIsCompatibleWithPipeline<Mesh, Pipeline>
		&& (ObjectDataIsCompatibleWithPipeline<ObjectData, Pipeline>
		|| std::same_as<ObjectData, std::nullopt_t>)
void Scene::create_render_object(
	std::string const & name,
	Mesh const & mesh,
	Pipeline const & pipeline,
	ObjectData const & object_data /*= std::nullopt*/)
{
	std::expected<std::shared_ptr<RenderObject>, GraphicsError> obj
		= m_renderer.CreateRenderObject(name, mesh.GetAssetId().m_index, pipeline.GetAssetId().m_index);
	if (!obj.has_value())
	{
		std::cout << "create_render_object: Failed to create obj. Error: " << obj.error().GetMessage();
		return;
	}

	if constexpr (!std::same_as<ObjectData, std::nullopt_t>)
		obj.value()->SetObjectData(&object_data);

	m_render_objs.push_back(obj.value());
}
