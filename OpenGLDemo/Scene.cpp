// Scene.cpp

module;

#include <array>
#include <filesystem>
#include <iostream>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

module Scene;

import GraphicsPipeline;
import Mesh;
import ObjLoader;
import PipelineBuilder;
import PlatformUtils;
import Vertex;

namespace
{
	template <IsVertex T>
	struct AssetId
	{
		using VertexT = T;
		int m_index = -1;
	};

	template <typename AssetId1, typename AssetId2>
	concept AssetsAreCompatible = std::same_as<typename AssetId1::VertexT, typename AssetId2::VertexT>;

	class GroundMesh
	{
	public:
		using VertexT = TextureVertex;

		static AssetId<VertexT> Create(Renderer & renderer);

	private:
		static Mesh create_ground_mesh();
	};

	auto GroundMesh::Create(Renderer & renderer)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		Mesh ground_mesh = create_ground_mesh();
		if (!ground_mesh.IsInitialized())
		{
			std::cout << "Failed to create GroundMesh" << std::endl;
			return id;
		}

		id.m_index = renderer.AddMesh(std::move(ground_mesh));
		return id;
	}

	Mesh GroundMesh::create_ground_mesh()
	{
		float scale = 30.0f;

		static_assert(std::same_as<VertexT, TextureVertex>);
		std::vector<TextureVertex> verts{
			{ { -scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0 } },
			{ {  scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 1.0 } },
			{ { -scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0 } },
			{ {  scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0 } } };

		//static_assert(std::same_as<VertexT, ColorVertex>);
		//std::vector<ColorVertex> verts{
		//	{ { -scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 } },
		//	{ {  scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0 } },
		//	{ { -scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		//	{ {  scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.5, 0.5, 0.5 } } };

		std::vector<Mesh::IndexT> indices{
			1, 0, 2,
			1, 2, 3 };

		return Mesh{ verts, indices };
	}

	class SkyboxMesh
	{
	public:
		using VertexT = PositionVertex;

		static AssetId<VertexT> Create(Renderer & renderer);

	private:
		static Mesh create_skybox_mesh();
	};

	auto SkyboxMesh::Create(Renderer & renderer)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		Mesh skybox_mesh = create_skybox_mesh();
		if (!skybox_mesh.IsInitialized())
		{
			std::cout << "Failed to create SkyboxMesh" << std::endl;
			return id;
		}

		id.m_index = renderer.AddMesh(std::move(skybox_mesh));
		return id;
	}

	Mesh SkyboxMesh::create_skybox_mesh()
	{
		static_assert(std::same_as<VertexT, PositionVertex>);
		std::vector<PositionVertex> verts{
			{ { -1.0f,  1.0f, -1.0f } },
			{ { -1.0f, -1.0f, -1.0f } },
			{ {  1.0f, -1.0f, -1.0f } },
			{ {  1.0f,  1.0f, -1.0f } },
			{ { -1.0f,  1.0f,  1.0f } },
			{ { -1.0f, -1.0f,  1.0f } },
			{ {  1.0f, -1.0f,  1.0f } },
			{ {  1.0f,  1.0f,  1.0f } } };

		std::vector<Mesh::IndexT> indices{
			0, 1, 2,
			2, 3, 0,
			5, 1, 0,
			0, 4, 5,
			2, 6, 7,
			7, 3, 2,
			5, 4, 7,
			7, 6, 5,
			0, 3, 7,
			7, 4, 0,
			1, 5, 2,
			2, 5, 6 };

		return Mesh{ verts, indices };
	}

	class FileMesh
	{
	public:
		using VertexT = NormalVertex;

		static AssetId<VertexT> Create(
			Renderer & renderer,
			std::filesystem::path const & file_path);

	private:
		static std::optional<Mesh> create_mesh_from_file(
			std::filesystem::path const & file_path);
	};

	auto FileMesh::Create(
		Renderer & renderer,
		std::filesystem::path const & file_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<Mesh> file_mesh = create_mesh_from_file(file_path);
		if (!file_mesh.has_value())
			return id;

		id.m_index = renderer.AddMesh(std::move(file_mesh.value()));
		return id;
	}

	std::optional<Mesh> FileMesh::create_mesh_from_file(
		std::filesystem::path const & file_path)
	{
		if (!std::filesystem::exists(file_path))
		{
			std::cout << "FileMesh::create_mesh_from_file() file does not exist:" << file_path << std::endl;
			return std::nullopt;
		}

		std::vector<NormalVertex> verts;
		std::vector<Mesh::IndexT> indices;
		if (!ObjLoader::LoadObjFile(file_path, verts, indices))
		{
			std::cout << "create_mesh_from_file() error loading file:" << file_path << std::endl;
			return std::nullopt;
		}

		return Mesh{ verts, indices };
	}

	class TexturePipeline
	{
	public:
		using VertexT = TextureVertex;

		static AssetId<VertexT> Create(
			Renderer & renderer,
			Scene const & scene,
			std::filesystem::path const & shaders_path);

	private:
		static std::optional<GraphicsPipeline> create_texture_pipeline(
			Scene const & scene,
			std::filesystem::path const & shaders_path);
	};

	auto TexturePipeline::Create(
		Renderer & renderer,
		Scene const & scene,
		std::filesystem::path const & shaders_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<GraphicsPipeline> pipeline = create_texture_pipeline(scene, shaders_path);
		if (!pipeline.has_value())
		{
			std::cout << "Failed to create TexturePipeline" << std::endl;
			return id;
		}

		id.m_index = renderer.AddPipeline(std::move(pipeline.value()));
		return id;
	}

	std::optional<GraphicsPipeline> TexturePipeline::create_texture_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path)
	{
		PipelineBuilder builder;
		builder.LoadShaders(
			shaders_path / "texture.vert",
			shaders_path / "texture.frag");

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform("view_transform", scene.GetCamera().GetViewTransform());
				pipeline.SetUniform("proj_transform", scene.GetCamera().GetProjTransform());

				pipeline.SetUniform("ambient_light_color", scene.GetAmbientLight().m_color);
				pipeline.SetUniform("pointlight_1.pos", scene.GetPointLight1().m_pos);
				pipeline.SetUniform("pointlight_1.color", scene.GetPointLight1().m_color);
				pipeline.SetUniform("pointlight_1.radius", scene.GetPointLight1().m_radius);
				pipeline.SetUniform("pointlight_2.pos", scene.GetPointLight2().m_pos);
				pipeline.SetUniform("pointlight_2.color", scene.GetPointLight2().m_color);
				pipeline.SetUniform("pointlight_2.radius", scene.GetPointLight2().m_radius);
				pipeline.SetUniform("pointlight_3.pos", scene.GetPointLight3().m_pos);
				pipeline.SetUniform("pointlight_3.color", scene.GetPointLight3().m_color);
				pipeline.SetUniform("pointlight_3.radius", scene.GetPointLight3().m_radius);
				pipeline.SetUniform("spotlight_1.pos", scene.GetSpotLight().m_pos);
				pipeline.SetUniform("spotlight_1.dir", scene.GetSpotLight().m_dir);
				pipeline.SetUniform("spotlight_1.color", scene.GetSpotLight().m_color);
				pipeline.SetUniform("spotlight_1.inner_radius", scene.GetSpotLight().m_inner_radius);
				pipeline.SetUniform("spotlight_1.outer_radius", scene.GetSpotLight().m_outer_radius);
			});
		builder.SetPerObjectConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetUniform("model_transform", obj.GetModelTransform());

				int tex_id = obj.GetTextureId();
				if (tex_id != -1)
					scene.GetTexture(tex_id).Bind();
			});

		return builder.CreatePipeline();
	}

	class ReflectionPipeline
	{
	public:
		using VertexT = NormalVertex;

		static AssetId<VertexT> Create(
			Renderer & renderer,
			Scene const & scene,
			std::filesystem::path const & shaders_path);

	private:
		static std::optional<GraphicsPipeline> create_reflection_pipeline(
			Scene const & scene,
			std::filesystem::path const & shaders_path);
	};

	auto ReflectionPipeline::Create(
		Renderer & renderer,
		Scene const & scene,
		std::filesystem::path const & shaders_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<GraphicsPipeline> pipeline = create_reflection_pipeline(scene, shaders_path);
		if (!pipeline.has_value())
		{
			std::cout << "Failed to create ReflectionPipeline" << std::endl;
			return id;
		}

		id.m_index = renderer.AddPipeline(std::move(pipeline.value()));
		return id;
	}

	std::optional<GraphicsPipeline> ReflectionPipeline::create_reflection_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path)
	{
		PipelineBuilder builder;
		builder.LoadShaders(
			shaders_path / "reflection.vert",
			shaders_path / "reflection.frag");

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform("view_transform", scene.GetCamera().GetViewTransform());
				pipeline.SetUniform("proj_transform", scene.GetCamera().GetProjTransform());

				pipeline.SetUniform("ambient_light_color", scene.GetAmbientLight().m_color);
				pipeline.SetUniform("pointlight_1.pos", scene.GetPointLight1().m_pos);
				pipeline.SetUniform("pointlight_1.color", scene.GetPointLight1().m_color);
				pipeline.SetUniform("pointlight_1.radius", scene.GetPointLight1().m_radius);
				pipeline.SetUniform("pointlight_2.pos", scene.GetPointLight2().m_pos);
				pipeline.SetUniform("pointlight_2.color", scene.GetPointLight2().m_color);
				pipeline.SetUniform("pointlight_2.radius", scene.GetPointLight2().m_radius);
				pipeline.SetUniform("pointlight_3.pos", scene.GetPointLight3().m_pos);
				pipeline.SetUniform("pointlight_3.color", scene.GetPointLight3().m_color);
				pipeline.SetUniform("pointlight_3.radius", scene.GetPointLight3().m_radius);
				pipeline.SetUniform("spotlight_1.pos", scene.GetSpotLight().m_pos);
				pipeline.SetUniform("spotlight_1.dir", scene.GetSpotLight().m_dir);
				pipeline.SetUniform("spotlight_1.color", scene.GetSpotLight().m_color);
				pipeline.SetUniform("spotlight_1.inner_radius", scene.GetSpotLight().m_inner_radius);
				pipeline.SetUniform("spotlight_1.outer_radius", scene.GetSpotLight().m_outer_radius);

				pipeline.SetUniform("camera_pos_world", scene.GetCamera().GetPos());
			});
		builder.SetPerObjectConstantsCallback(
			[](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetUniform("model_transform", obj.GetModelTransform());
			});

		return builder.CreatePipeline();
	}

	class SkyboxPipeline
	{
	public:
		using VertexT = PositionVertex;

		static AssetId<VertexT> Create(
			Renderer & renderer,
			Scene const & scene,
			std::filesystem::path const & shaders_path);

	private:
		static std::optional<GraphicsPipeline> create_skybox_pipeline(
			Scene const & scene,
			std::filesystem::path const & shaders_path);
	};

	auto SkyboxPipeline::Create(
		Renderer & renderer,
		Scene const & scene,
		std::filesystem::path const & shaders_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<GraphicsPipeline> pipeline = create_skybox_pipeline(scene, shaders_path);
		if (!pipeline.has_value())
		{
			std::cout << "Failed to create SkyboxPipeline" << std::endl;
			return id;
		}

		id.m_index = renderer.AddPipeline(std::move(pipeline.value()));
		return id;
	}

	std::optional<GraphicsPipeline> SkyboxPipeline::create_skybox_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path)
	{
		PipelineBuilder builder;
		builder.LoadShaders(
			shaders_path / "skybox.vert",
			shaders_path / "skybox.frag");
		builder.SetDepthTestOptions(DepthTestOptions{
			.m_enable_depth_test = true,
			.m_enable_depth_write = false,
			.m_depth_compare_op = DepthCompareOp::EQUAL
			});

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform("view_transform", scene.GetCamera().GetViewTransform());
				pipeline.SetUniform("proj_transform", scene.GetCamera().GetProjTransform());
			});

		return builder.CreatePipeline();
	}

	class ColorPipeline
	{
	public:
		using VertexT = ColorVertex;

		static AssetId<VertexT> Create(
			Renderer & renderer,
			Scene const & scene,
			std::filesystem::path const & shaders_path);

	private:
		static std::optional<GraphicsPipeline> create_color_pipeline(
			Scene const & scene,
			std::filesystem::path const & shaders_path);
	};

	auto ColorPipeline::Create(
		Renderer & renderer,
		Scene const & scene,
		std::filesystem::path const & shaders_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<GraphicsPipeline> pipeline = create_color_pipeline(scene, shaders_path);
		if (!pipeline.has_value())
		{
			std::cout << "Failed to create ColorPipeline" << std::endl;
			return id;
		}

		id.m_index = renderer.AddPipeline(std::move(pipeline.value()));
		return id;
	}

	std::optional<GraphicsPipeline> ColorPipeline::create_color_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path)
	{
		PipelineBuilder builder;
		builder.LoadShaders(
			shaders_path / "color.vert",
			shaders_path / "color.frag");

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform("view_transform", scene.GetCamera().GetViewTransform());
				pipeline.SetUniform("proj_transform", scene.GetCamera().GetProjTransform());

				pipeline.SetUniform("ambient_light_color", scene.GetAmbientLight().m_color);
				pipeline.SetUniform("pointlight_1.pos", scene.GetPointLight1().m_pos);
				pipeline.SetUniform("pointlight_1.color", scene.GetPointLight1().m_color);
				pipeline.SetUniform("pointlight_1.radius", scene.GetPointLight1().m_radius);
				pipeline.SetUniform("pointlight_2.pos", scene.GetPointLight2().m_pos);
				pipeline.SetUniform("pointlight_2.color", scene.GetPointLight2().m_color);
				pipeline.SetUniform("pointlight_2.radius", scene.GetPointLight2().m_radius);
				pipeline.SetUniform("pointlight_3.pos", scene.GetPointLight3().m_pos);
				pipeline.SetUniform("pointlight_3.color", scene.GetPointLight3().m_color);
				pipeline.SetUniform("pointlight_3.radius", scene.GetPointLight3().m_radius);
				pipeline.SetUniform("spotlight_1.pos", scene.GetSpotLight().m_pos);
				pipeline.SetUniform("spotlight_1.dir", scene.GetSpotLight().m_dir);
				pipeline.SetUniform("spotlight_1.color", scene.GetSpotLight().m_color);
				pipeline.SetUniform("spotlight_1.inner_radius", scene.GetSpotLight().m_inner_radius);
				pipeline.SetUniform("spotlight_1.outer_radius", scene.GetSpotLight().m_outer_radius);
			});
		builder.SetPerObjectConstantsCallback(
			[](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetUniform("model_transform", obj.GetModelTransform());
			});

		return builder.CreatePipeline();
	}

	class LightSourcePipeline
	{
	public:
		using VertexT = NormalVertex;

		static AssetId<VertexT> Create(
			Renderer & renderer,
			Scene const & scene,
			std::filesystem::path const & shaders_path);

	private:
		static std::optional<GraphicsPipeline> create_light_source_pipeline(
			Scene const & scene,
			std::filesystem::path const & shaders_path);
	};

	auto LightSourcePipeline::Create(
		Renderer & renderer,
		Scene const & scene,
		std::filesystem::path const & shaders_path)
		-> AssetId<VertexT>
	{
		AssetId<VertexT> id;

		std::optional<GraphicsPipeline> pipeline = create_light_source_pipeline(scene, shaders_path);
		if (!pipeline.has_value())
		{
			std::cout << "Failed to create LightSourcePipeline" << std::endl;
			return id;
		}

		id.m_index = renderer.AddPipeline(std::move(pipeline.value()));
		return id;
	}

	std::optional<GraphicsPipeline> LightSourcePipeline::create_light_source_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path)
	{
		PipelineBuilder builder;
		builder.LoadShaders(
			shaders_path / "light_source.vert",
			shaders_path / "light_source.frag");

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform("view_transform", scene.GetCamera().GetViewTransform());
				pipeline.SetUniform("proj_transform", scene.GetCamera().GetProjTransform());

				pipeline.SetUniform("camera_pos_world", scene.GetCamera().GetPos());
			});
		builder.SetPerObjectConstantsCallback(
			[](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetUniform("model_transform", obj.GetModelTransform());

				pipeline.SetUniform("object_color", obj.GetColor());
			});

		return builder.CreatePipeline();
	}

	template <typename MeshAssetId, typename PipelineAssetId>
		requires AssetsAreCompatible<MeshAssetId, PipelineAssetId>
	std::shared_ptr<RenderObject> create_render_object(
		Renderer & renderer,
		std::string const & name,
		MeshAssetId mesh_id,
		PipelineAssetId pipeline_id,
		int tex_id = -1)
	{
		return renderer.CreateRenderObject(name, mesh_id.m_index, pipeline_id.m_index, tex_id);
	}

	void init_sword_transform(int index, glm::mat4 & transform)
	{
		float x_rot = static_cast<float>(std::numbers::pi / 2.0);
		float y_rot = static_cast<float>(std::numbers::pi / 4.0);
		glm::vec3 pos(0.0f, 0.5f, 3.5f);
		if (index == 1)
		{
			y_rot = -y_rot;
			pos.y = -pos.y;
		}

		// the sword mesh was designed for the Y axis being the up direction, but we're using the Z axis, this can be corrected by rotating on the X axis
		transform = glm::rotate(glm::mat4(1.0), x_rot, glm::vec3(1.0, 0.0, 0.0)) * transform;
		transform = glm::rotate(glm::mat4(1.0), y_rot, glm::vec3(0.0, 1.0, 0.0)) * transform;
		transform = glm::translate(glm::mat4(1.0), pos) * transform;
	}

	void update_sword_transform(int index, glm::mat4 & transform, float time, float delta_time)
	{
		float sword_pos_z;
		if (index == 0)
			sword_pos_z = std::cos(time * 0.5f) * 0.5f + 3.5f;
		else
			sword_pos_z = std::sin(time * 0.5f) * 0.5f + 3.5f;
		transform[3][2] = sword_pos_z;

		glm::vec3 sword_pos(transform[3][0], transform[3][1], transform[3][2]);
		glm::vec3 sword_y_axis(transform[1][0], transform[1][1], transform[1][2]);

		transform = glm::translate(glm::mat4(1.0), -sword_pos) * transform;
		transform = glm::rotate(glm::mat4(1.0), delta_time * 0.5f, sword_y_axis) * transform;
		transform = glm::translate(glm::mat4(1.0), sword_pos) * transform;
	}

	void init_gem_transform(int index, glm::mat4 & transform)
	{
		float x_rot = static_cast<float>(std::numbers::pi / 2.0);
		float z_rot = static_cast<float>(index * std::numbers::pi * 2.0 / 3.0);
		glm::vec3 pos(0.0f, 4.0f, 2.0f);

		// the gem meshes were designed for the Y axis being the up direction, but we're using the Z axis, this can be corrected by rotating on the X axis
		transform = glm::rotate(glm::mat4(1.0), x_rot, glm::vec3(1.0, 0.0, 0.0)) * transform;
		transform = glm::translate(glm::mat4(1.0), pos) * transform;
		transform = glm::rotate(glm::mat4(1.0), z_rot, glm::vec3(0.0, 0.0, 1.0)) * transform;
	}

	void update_gem_transform(glm::mat4 & transform, float delta_time)
	{
		transform = glm::rotate(glm::mat4(1.0), delta_time * 0.5f, glm::vec3(0.0, 0.0, 1.0)) * transform;
	}
}

void Scene::Init()
{
	const std::filesystem::path resources_path = PlatformUtils::GetExecutableDir() / "resources";
	const std::filesystem::path shaders_path = resources_path / "shaders";
	const std::filesystem::path textures_path = resources_path / "textures";
	const std::filesystem::path objects_path = resources_path / "objects";

	int ground_tex_id = static_cast<int>(m_textures.size());
	m_textures.emplace_back(textures_path / "skybox" / "top.jpg");

	int skybox_tex_id = static_cast<int>(m_textures.size());
	m_textures.emplace_back(std::array<std::filesystem::path, 6>{
		textures_path / "skybox" / "right.jpg",
		textures_path / "skybox" / "left.jpg",
		textures_path / "skybox" / "top.jpg",
		textures_path / "skybox" / "bottom.jpg",
		textures_path / "skybox" / "front.jpg",
		textures_path / "skybox" / "back.jpg"
	});

	AssetId<TexturePipeline::VertexT> texture_pipeline_id = TexturePipeline::Create(m_renderer, *this, shaders_path);
	AssetId<LightSourcePipeline::VertexT> light_source_pipeline_id = LightSourcePipeline::Create(m_renderer, *this, shaders_path);
	AssetId<ReflectionPipeline::VertexT> reflection_pipeline_id = ReflectionPipeline::Create(m_renderer, *this, shaders_path);
	AssetId<SkyboxPipeline::VertexT> skybox_pipeline_id = SkyboxPipeline::Create(m_renderer, *this, shaders_path);
	//AssetId<ColorPipeline::VertexT> color_pipeline_id = ColorPipeline::Create(m_renderer, *this, shaders_path);

	AssetId<FileMesh::VertexT> sword_mesh_id = FileMesh::Create(m_renderer,
		objects_path / "skullsword.obj");
	AssetId<FileMesh::VertexT> red_gem_mesh_id = FileMesh::Create(m_renderer,
		objects_path / "redgem.obj");
	AssetId<FileMesh::VertexT> green_gem_mesh_id = FileMesh::Create(m_renderer,
		objects_path / "greengem.obj");
	AssetId<FileMesh::VertexT> blue_gem_mesh_id = FileMesh::Create(m_renderer,
		objects_path / "bluegem.obj");
	AssetId<GroundMesh::VertexT> ground_mesh_id = GroundMesh::Create(m_renderer);
	AssetId<SkyboxMesh::VertexT> skybox_mesh_id = SkyboxMesh::Create(m_renderer);

	m_sword0 = create_render_object(m_renderer, "sword0", sword_mesh_id, reflection_pipeline_id, skybox_tex_id);
	m_sword1 = create_render_object(m_renderer, "sword1", sword_mesh_id, reflection_pipeline_id, skybox_tex_id);
	m_red_gem = create_render_object(m_renderer, "red gem", red_gem_mesh_id, light_source_pipeline_id);
	m_green_gem = create_render_object(m_renderer, "green gem", green_gem_mesh_id, light_source_pipeline_id);
	m_blue_gem = create_render_object(m_renderer, "blue gem", blue_gem_mesh_id, light_source_pipeline_id);
	m_ground = create_render_object(m_renderer, "ground", ground_mesh_id, texture_pipeline_id, ground_tex_id);
	m_skybox = create_render_object(m_renderer, "skybox", skybox_mesh_id, skybox_pipeline_id, skybox_tex_id);

	m_red_gem->SetColor({ 1.0, 0.0, 0.0 });
	m_green_gem->SetColor({ 0.0, 1.0, 0.0 });
	m_blue_gem->SetColor({ 0.0, 0.0, 1.0 });

	init_sword_transform(0, m_sword0->ModifyModelTransform());
	init_sword_transform(1, m_sword1->ModifyModelTransform());
	init_gem_transform(0, m_red_gem->ModifyModelTransform());
	init_gem_transform(1, m_green_gem->ModifyModelTransform());
	init_gem_transform(2, m_blue_gem->ModifyModelTransform());

	m_ambient_light = AmbientLight{ glm::vec3{ 0.5, 0.5, 0.5 } };

	m_spotlight = SpotLight{
		.m_pos{ 0.0f, 0.0f, 25.0f },
		.m_dir{ 0.0f, 0.0f, -1.0f },
		.m_color{ 1.0f, 1.0f, 1.0f },
		.m_inner_radius = 0.988f,
		.m_outer_radius = 0.986f
	};

	glm::vec3 camera_pos{ 0.0f, -10.0f, 5.0f };
	glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 0.0f, 2.5f } - camera_pos);
	m_camera.Init(camera_pos, camera_dir);
}

void Scene::OnViewportResized(int width, int height)
{
	m_camera.OnViewportResized(width, height);
}

void Scene::Update(double delta_time, Input const & input)
{
	const float dt = static_cast<float>(delta_time);
	m_timer += dt;

	m_camera.Update(delta_time, input);

	glm::vec3 bg_color;
	bg_color.r = std::sin(m_timer) / 2.0f + 0.5f;
	bg_color.g = std::cos(m_timer) / 2.0f + 0.5f;
	bg_color.b = std::tan(m_timer) / 2.0f + 0.5f;
	m_renderer.SetClearColor(bg_color);

	update_sword_transform(0, m_sword0->ModifyModelTransform(), m_timer, dt);
	update_sword_transform(1, m_sword1->ModifyModelTransform(), m_timer, dt);
	update_gem_transform(m_red_gem->ModifyModelTransform(), dt);
	update_gem_transform(m_green_gem->ModifyModelTransform(), dt);
	update_gem_transform(m_blue_gem->ModifyModelTransform(), dt);

	glm::mat4 const & red_gem_transform = m_red_gem->GetModelTransform();
	m_pointlight_1 = PointLight{
		.m_pos{ red_gem_transform[3][0], red_gem_transform[3][1], red_gem_transform[3][2] },
		.m_color{ 1.0, 0.0, 0.0 },
		.m_radius = 20.0f
	};

	glm::mat4 const & green_gem_transform = m_green_gem->GetModelTransform();
	m_pointlight_2 = PointLight{
		.m_pos{ green_gem_transform[3][0], green_gem_transform[3][1], green_gem_transform[3][2] },
		.m_color{ 0.0, 1.0, 0.0 },
		.m_radius = 20.0f
	};

	glm::mat4 const & blue_gem_transform = m_blue_gem->GetModelTransform();
	m_pointlight_3 = PointLight{
		.m_pos{ blue_gem_transform[3][0], blue_gem_transform[3][1], blue_gem_transform[3][2] },
		.m_color{ 0.0, 0.0, 1.0 },
		.m_radius = 20.0f
	};
}

void Scene::Render() const
{
	m_renderer.Render();
}

Texture const & Scene::GetTexture(int id) const
{
	if (id < 0 || id >= static_cast<int>(m_textures.size()))
		throw std::out_of_range("Invalid texture ID");
	return m_textures[id];
}
