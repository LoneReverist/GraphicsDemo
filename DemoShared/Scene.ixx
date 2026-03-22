// Scene.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

export module Scene;

import AssetPool;
import Camera;
import ColorPipeline;
import FontAtlas;
import GraphicsApi;
import GraphicsError;
import GraphicsPipeline;
import Input;
import LightsManager;
import LightSourcePipeline;
import Mesh;
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

template <typename Mesh, typename Pipeline>
concept MeshIsCompatibleWithPipeline = std::same_as<typename Mesh::VertexT, typename Pipeline::VertexT>;

template <typename Pipeline>
concept PipelineHasObjectData = requires { typename Pipeline::ObjectData; };

template <typename ObjectData, typename Pipeline>
concept ObjectDataIsCompatibleWithPipeline = std::same_as<ObjectData, typename Pipeline::ObjectData>
|| (!PipelineHasObjectData<Pipeline> && std::same_as<ObjectData, std::nullopt_t>);

// Keeps track of which render objects are using the associated pipeline,
// this allows the render objects to be grouped by pipeline for more efficient rendering
struct PipelineRenderObjects
{
	AssetId pipeline_id;
	std::vector<AssetId> render_object_ids;
};

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
		requires MeshIsCompatibleWithPipeline<Mesh, Pipeline> && ObjectDataIsCompatibleWithPipeline<ObjectData, Pipeline>
	AssetId create_render_object(
		std::string const & name,
		Mesh const & mesh,
		Pipeline const & pipeline,
		ObjectData const & object_data = std::nullopt);

	AssetId create_texture(std::filesystem::path const & filepath);
	AssetId create_cubemap_texture(std::array<std::filesystem::path, 6> const & filepaths);

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
	LightsManager m_lights;

	AssetPool<Mesh> m_mesh_pool;
	AssetPool<GraphicsPipeline> m_pipeline_pool;
	AssetPool<RenderObject> m_render_object_pool;
	AssetPool<Texture> m_texture_pool;

	std::vector<PipelineRenderObjects> m_active_render_objects;

	std::unique_ptr<FontAtlas> m_arial_font;

	std::unique_ptr<TextMesh> m_fps_mesh;
	std::unique_ptr<TextMesh> m_title_mesh;

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

	float m_timer = 0.0f;
	float m_frame_timer = 0.0f;
	int m_frame_count = 0;

	float m_dpi_scale = 1.0f;
};

template<IsVertex VertexT, typename... Args>
MeshAsset<VertexT> Scene::create_mesh(Args &&... args)
{
	std::expected<Mesh, GraphicsError> mesh
		= MeshAsset<VertexT>::Create(m_graphics_api, std::forward<Args>(args)...);
	if (!mesh.has_value())
	{
		std::cout << "Failed to create MeshAsset<" << typeid(VertexT).name()
			<< "> Error: " << mesh.error().GetMessage() << std::endl;
		return MeshAsset<VertexT>{};
	}

	AssetId mesh_id = m_mesh_pool.Add(std::move(mesh.value()));
	if (!mesh_id.IsValid())
		std::cout << "Failed to add mesh to pool." << std::endl;

	return MeshAsset<VertexT>{ mesh_id };
}

template <typename PipelineT, typename... Args>
PipelineT Scene::create_pipeline(Args &&... args)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";

	std::expected<GraphicsPipeline, GraphicsError> pipeline
		= PipelineT::CreateGraphicsPipeline(m_graphics_api, shaders_path, std::forward<Args>(args)...);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create " << typeid(PipelineT).name()
			<< " Error: " << pipeline.error().GetMessage() << std::endl;
		return PipelineT{};
	}

	AssetId pipeline_id = m_pipeline_pool.Add(std::move(pipeline.value()));
	if (!pipeline_id.IsValid())
		std::cout << "Failed to add pipeline to pool." << std::endl;

	return PipelineT{ pipeline_id };
}

template <typename Mesh, typename Pipeline, typename ObjectData /*= std::nullopt_t*/>
	requires MeshIsCompatibleWithPipeline<Mesh, Pipeline> && ObjectDataIsCompatibleWithPipeline<ObjectData, Pipeline>
AssetId Scene::create_render_object(
	std::string const & name,
	Mesh const & mesh,
	Pipeline const & pipeline,
	ObjectData const & object_data /*= std::nullopt*/)
{
	if (!mesh.GetAssetId().IsValid())
	{
		std::cout << "Scene::create_render_object: invalid mesh id for object: " + name;
		return AssetId{};
	}
	if (!pipeline.GetAssetId().IsValid())
	{
		std::cout << "Scene::create_render_object: invalid pipeline id for object: " + name;
		return AssetId{};
	}

	RenderObject obj{ name, mesh.GetAssetId(), pipeline.GetAssetId() };
	if constexpr (!std::same_as<ObjectData, std::nullopt_t>)
		obj.SetObjectData(&object_data);

	AssetId obj_id = m_render_object_pool.Add(std::move(obj));
	if (!obj_id.IsValid())
	{
		std::cout << "Failed to add render object to pool for object: " + name;
		return obj_id;
	}

	auto iter = std::ranges::find(m_active_render_objects, pipeline.GetAssetId(), &PipelineRenderObjects::pipeline_id);
	if (iter != m_active_render_objects.end())
		iter->render_object_ids.push_back(obj_id);
	else
		m_active_render_objects.push_back(PipelineRenderObjects{ pipeline.GetAssetId(), { obj_id } });

	return obj_id;
}
