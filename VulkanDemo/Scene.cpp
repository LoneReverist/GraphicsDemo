// Scene.cpp

module;

#include <array>
#include <filesystem>
#include <iostream>
#include <numbers>
#include <optional>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

module Scene;

import AssetId;
import GraphicsPipeline;
import Mesh;
import ObjLoader;
import PlatformUtils;
import StbImage;
import Vertex;

std::unique_ptr<Texture> create_texture(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & filepath)
{
	PixelFormat format = PixelFormat::RGBA_SRGB;

	StbImage image(filepath, GetPixelSize(format) /*STBI_rgb_alpha*/);
	if (!image.IsValid())
	{
		std::cout << "Failed to load image: " << filepath << std::endl;
		return nullptr;
	}

	return std::make_unique<Texture>(
		graphics_api,
		ImageData{
			.m_data = image.GetData(),
			.m_format = format,
			.m_width = static_cast<std::uint32_t>(image.GetWidth()),
			.m_height = static_cast<std::uint32_t>(image.GetHeight())
		});
}

std::unique_ptr<Texture> create_cubemap_texture(
	GraphicsApi const & graphics_api,
	std::array<std::filesystem::path, 6> const & filepaths)
{
	PixelFormat format = PixelFormat::RGBA_SRGB;

	int width = 0, height = 0;
	std::array<StbImage, 6> images;
	for (size_t i = 0; i < filepaths.size(); ++i)
	{
		images[i].LoadImage(filepaths[i], GetPixelSize(format) /*STBI_rgb_alpha*/);
		if (!images[i].IsValid())
		{
			std::cout << "Failed to load cubemap image: " << filepaths[i] << std::endl;
			return nullptr;
		}

		if (i == 0)
		{
			width = images[i].GetWidth();
			height = images[i].GetHeight();
		}
		else if (images[i].GetWidth() != width || images[i].GetHeight() != height)
		{
			std::cout << "Cubemap images must have the same dimensions." << std::endl;
			return nullptr;
		}
	}

	std::array<std::uint8_t const *, 6> data;
	std::ranges::transform(images, data.begin(),
		[](StbImage const & img) { return img.GetData(); });

	return std::make_unique<Texture>(
		graphics_api,
		CubeImageData{
			.m_data = data,
			.m_format = format,
			.m_width = static_cast<std::uint32_t>(width),
			.m_height = static_cast<std::uint32_t>(height)
		});
}

class GroundMesh
{
public:
	using VertexT = TextureVertex;

	static AssetId<VertexT> Create(
		GraphicsApi const & graphics_api,
		Renderer & renderer);

private:
	static Mesh create_ground_mesh(GraphicsApi const & graphics_api);
};

auto GroundMesh::Create(
	GraphicsApi const & graphics_api,
	Renderer & renderer)
	-> AssetId<VertexT>
{
	AssetId<VertexT> id;

	Mesh ground_mesh = create_ground_mesh(graphics_api);
	if (!ground_mesh.IsInitialized())
	{
		std::cout << "Failed to create GroundMesh" << std::endl;
		return id;
	}

	id.m_index = renderer.AddMesh(std::move(ground_mesh));
	return id;
}

Mesh GroundMesh::create_ground_mesh(GraphicsApi const & graphics_api)
{
	float scale = 30.0f;

	static_assert(std::same_as<VertexT, TextureVertex>);
	std::vector<TextureVertex> verts {
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

	return Mesh{ graphics_api, verts, indices };
}

class SkyboxMesh
{
public:
	using VertexT = PositionVertex;

	static AssetId<VertexT> Create(
		GraphicsApi const & graphics_api,
		Renderer & renderer);

private:
	static Mesh create_skybox_mesh(GraphicsApi const & graphics_api);
};

auto SkyboxMesh::Create(
	GraphicsApi const & graphics_api,
	Renderer & renderer)
	-> AssetId<VertexT>
{
	AssetId<VertexT> id;

	Mesh skybox_mesh = create_skybox_mesh(graphics_api);
	if (!skybox_mesh.IsInitialized())
	{
		std::cout << "Failed to create SkyboxMesh" << std::endl;
		return id;
	}

	id.m_index = renderer.AddMesh(std::move(skybox_mesh));
	return id;
}

Mesh SkyboxMesh::create_skybox_mesh(GraphicsApi const & graphics_api)
{
	static_assert(std::same_as<VertexT, PositionVertex>);
	std::vector<PositionVertex> verts {
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

	return Mesh{ graphics_api, verts, indices };
}

class FileMesh
{
public:
	using VertexT = NormalVertex;

	static AssetId<VertexT> Create(
		GraphicsApi const & graphics_api,
		Renderer & renderer,
		std::filesystem::path const & file_path);

private:
	static std::optional<Mesh> create_mesh_from_file(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & file_path);
};

auto FileMesh::Create(
	GraphicsApi const & graphics_api,
	Renderer & renderer,
	std::filesystem::path const & file_path)
	-> AssetId<VertexT>
{
	AssetId<VertexT> id;

	std::optional<Mesh> file_mesh = create_mesh_from_file(graphics_api, file_path);
	if (!file_mesh.has_value())
		return id;

	id.m_index = renderer.AddMesh(std::move(file_mesh.value()));
	return id;
}

std::optional<Mesh> FileMesh::create_mesh_from_file(
	GraphicsApi const & graphics_api,
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

	return Mesh{ graphics_api, verts, indices };
}

ColorPipeline Scene::create_color_pipeline()
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";
	std::optional<GraphicsPipeline> pipeline = ColorPipeline::CreateGraphicsPipeline(
		m_graphics_api, shaders_path, m_camera, m_lights);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create ColorPipeline" << std::endl;
		return ColorPipeline{};
	}
	AssetId<ColorPipeline::VertexT> asset_id{ m_renderer.AddPipeline(std::move(pipeline.value())) };
	return ColorPipeline{ asset_id };
}

TexturePipeline Scene::create_texture_pipeline(Texture const & texture)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";
	std::optional<GraphicsPipeline> pipeline = TexturePipeline::CreateGraphicsPipeline(
		m_graphics_api, shaders_path, m_camera, m_lights, texture);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create TexturePipeline" << std::endl;
		return TexturePipeline{};
	}
	AssetId<TexturePipeline::VertexT> asset_id{ m_renderer.AddPipeline(std::move(pipeline.value())) };
	return TexturePipeline{ asset_id };
}

SkyboxPipeline Scene::create_skybox_pipeline(Texture const & texture)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";
	std::optional<GraphicsPipeline> pipeline = SkyboxPipeline::CreateGraphicsPipeline(
		m_graphics_api, shaders_path, m_camera, texture);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create SkyboxPipeline" << std::endl;
		return SkyboxPipeline{};
	}
	AssetId<SkyboxPipeline::VertexT> asset_id{ m_renderer.AddPipeline(std::move(pipeline.value())) };
	return SkyboxPipeline{ asset_id };
}

LightSourcePipeline Scene::create_light_source_pipeline()
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";
	std::optional<GraphicsPipeline> pipeline = LightSourcePipeline::CreateGraphicsPipeline(
		m_graphics_api, shaders_path, m_camera);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create LightSourcePipeline" << std::endl;
		return LightSourcePipeline{};
	}
	AssetId<LightSourcePipeline::VertexT> asset_id{ m_renderer.AddPipeline(std::move(pipeline.value())) };
	return LightSourcePipeline{ asset_id };
}

ReflectionPipeline Scene::create_reflection_pipeline(Texture const & texture)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";
	std::optional<GraphicsPipeline> pipeline = ReflectionPipeline::CreateGraphicsPipeline(
		m_graphics_api, shaders_path, m_camera, m_lights, texture);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create ReflectionPipeline" << std::endl;
		return ReflectionPipeline{};
	}
	AssetId<ReflectionPipeline::VertexT> asset_id{ m_renderer.AddPipeline(std::move(pipeline.value())) };
	return ReflectionPipeline{ asset_id };
}

TextPipeline Scene::create_text_pipeline(FontAtlas const & font_atlas)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";
	std::optional<GraphicsPipeline> pipeline = TextPipeline::CreateGraphicsPipeline(
		m_graphics_api, shaders_path, font_atlas);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create TextPipeline" << std::endl;
		return TextPipeline{};
	}
	AssetId<TextPipeline::VertexT> asset_id{ m_renderer.AddPipeline(std::move(pipeline.value())) };
	return TextPipeline{ asset_id };
}

RainbowTextPipeline Scene::create_rainbow_text_pipeline(FontAtlas const & font_atlas)
{
	std::filesystem::path shaders_path = m_resources_path / "shaders";
	std::optional<GraphicsPipeline> pipeline = RainbowTextPipeline::CreateGraphicsPipeline(
		m_graphics_api, shaders_path, font_atlas);
	if (!pipeline.has_value())
	{
		std::cout << "Failed to create RainbowTextPipeline" << std::endl;
		return RainbowTextPipeline{};
	}
	AssetId<RainbowTextPipeline::VertexT> asset_id{ m_renderer.AddPipeline(std::move(pipeline.value())) };
	return RainbowTextPipeline{ asset_id };
}

template <typename ObjectData, typename Pipeline>
concept ObjectDataIsCompatibleWithPipeline = std::same_as<ObjectData, typename Pipeline::ObjectData>;

template <typename MeshAssetId, typename Pipeline, typename ObjectData>
	requires AssetsAreCompatible<MeshAssetId, typename Pipeline::AssetIdT>
	&& ObjectDataIsCompatibleWithPipeline<ObjectData, Pipeline>
std::shared_ptr<RenderObject> create_render_object(
	Renderer & renderer,
	std::string const & name,
	MeshAssetId mesh_id,
	Pipeline const & pipeline,
	ObjectData const & object_data)
{
	std::shared_ptr<RenderObject> obj = renderer.CreateRenderObject(name, mesh_id.m_index, pipeline.GetAssetId().m_index);
	obj->SetObjectData(&object_data);
	return obj;
}

template <typename MeshAssetId, typename Pipeline>
	requires AssetsAreCompatible<MeshAssetId, Pipeline>
std::shared_ptr<RenderObject> create_render_object(
	Renderer & renderer,
	std::string const & name,
	MeshAssetId mesh_id,
	Pipeline pipeline)
{
	return renderer.CreateRenderObject(name, mesh_id.m_index, pipeline.GetAssetId().m_index);
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

void Scene::Init()
{
	m_dpi_scale = PlatformUtils::GetDPIScalingFactor();
	float label_font_size = 18.0f * m_dpi_scale;
	float title_font_size = 32.0f * m_dpi_scale;

	m_resources_path = PlatformUtils::GetExecutableDir() / "resources";
	const std::filesystem::path textures_path = m_resources_path / "textures";
	const std::filesystem::path objects_path = m_resources_path / "objects";
	const std::filesystem::path fonts_path = m_resources_path / "fonts";

	m_ground_tex = create_texture(m_graphics_api,
		textures_path / "skybox" / "top.jpg");
	m_skybox_tex = create_cubemap_texture(m_graphics_api, std::array<std::filesystem::path, 6>{
		textures_path / "skybox" / "right.jpg",
		textures_path / "skybox" / "left.jpg",
		textures_path / "skybox" / "top.jpg",
		textures_path / "skybox" / "bottom.jpg",
		textures_path / "skybox" / "front.jpg",
		textures_path / "skybox" / "back.jpg"
	});

	m_arial_font = std::make_unique<FontAtlas>(m_graphics_api, fonts_path / "ArialAtlas.png", fonts_path / "ArialAtlas.json");

	//ColorPipeline color_pipeline = create_color_pipeline(shaders_path);
	TexturePipeline ground_pipeline = create_texture_pipeline(*m_ground_tex);
	SkyboxPipeline skybox_pipeline = create_skybox_pipeline(*m_skybox_tex);
	LightSourcePipeline light_source_pipeline = create_light_source_pipeline();
	ReflectionPipeline reflection_pipeline = create_reflection_pipeline(*m_skybox_tex);
	TextPipeline text_pipeline = create_text_pipeline(*m_arial_font);
	RainbowTextPipeline rainbow_text_pipeline = create_rainbow_text_pipeline(*m_arial_font);

	AssetId<FileMesh::VertexT> sword_mesh_id = FileMesh::Create(m_graphics_api, m_renderer,
		objects_path / "skullsword.obj");
	AssetId<FileMesh::VertexT> red_gem_mesh_id = FileMesh::Create(m_graphics_api, m_renderer,
		objects_path / "redgem.obj");
	AssetId<FileMesh::VertexT> green_gem_mesh_id = FileMesh::Create(m_graphics_api, m_renderer,
		objects_path / "greengem.obj");
	AssetId<FileMesh::VertexT> blue_gem_mesh_id = FileMesh::Create(m_graphics_api, m_renderer,
		objects_path / "bluegem.obj");
	AssetId<GroundMesh::VertexT> ground_mesh_id = GroundMesh::Create(m_graphics_api, m_renderer);
	AssetId<SkyboxMesh::VertexT> skybox_mesh_id = SkyboxMesh::Create(m_graphics_api, m_renderer);

	m_fps_mesh = std::make_unique<TextMesh>(TextMesh::Create(m_graphics_api, m_renderer,
		"FPS: ", *m_arial_font, label_font_size, glm::vec2{ -0.9, -0.9 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/, true /*flip_y*/));
	m_fps_label = TextPipeline::ObjectData{
		.m_screen_px_range = m_fps_mesh->GetScreenPxRange(),
		.m_bg_color = { 0.0f, 0.0f, 0.0f, 0.3f },
		.m_text_color = { 1.0f, 1.0f, 0.0f, 1.0 },
	};

	m_title_mesh = std::make_unique<TextMesh>(TextMesh::Create(m_graphics_api, m_renderer,
		"Vulkan Demo", *m_arial_font, title_font_size, glm::vec2{ -0.9, 0.8 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/, true /*flip_y*/));
	m_title_label = RainbowTextPipeline::ObjectData{
		.m_bg_color = { 0.0f, 0.0f, 0.0f, 0.3f },
		.m_screen_px_range = m_title_mesh->GetScreenPxRange(),
		.m_rainbow_width = 200.0f * m_dpi_scale,
		.m_slant_factor = -1.0f
	};

	m_red_gem.m_color = glm::vec3{ 1.0f, 0.0f, 0.0f };
	m_green_gem.m_color = glm::vec3{ 0.0f, 1.0f, 0.0f };
	m_blue_gem.m_color = glm::vec3{ 0.0f, 0.0f, 1.0f };

	init_sword_transform(0, m_sword0.m_model);
	init_sword_transform(1, m_sword1.m_model);
	init_gem_transform(0, m_red_gem.m_model);
	init_gem_transform(1, m_green_gem.m_model);
	init_gem_transform(2, m_blue_gem.m_model);

	m_lights.SetAmbientLight(AmbientLight{ glm::vec3{ 0.3, 0.3, 0.3 } });

	m_lights.SetSpotLight(SpotLight{
		.m_pos{ 0.0f, 0.0f, 25.0f },
		.m_dir{ 0.0f, 0.0f, -1.0f },
		.m_color{ 1.0f, 1.0f, 1.0f },
		.m_inner_radius = 0.988f,
		.m_outer_radius = 0.986f
		});

	glm::vec3 camera_pos{ 0.0f, -10.0f, 5.0f };
	glm::vec3 camera_dir = glm::normalize(glm::vec3{ 0.0f, 0.0f, 2.5f } - camera_pos);
	m_camera.Init(camera_pos, camera_dir);

	m_render_objs = {
		create_render_object(m_renderer, "sword0", sword_mesh_id, reflection_pipeline, m_sword0),
		create_render_object(m_renderer, "sword1", sword_mesh_id, reflection_pipeline, m_sword1),
		create_render_object(m_renderer, "red gem", red_gem_mesh_id, light_source_pipeline, m_red_gem),
		create_render_object(m_renderer, "green gem", green_gem_mesh_id, light_source_pipeline, m_green_gem),
		create_render_object(m_renderer, "blue gem", blue_gem_mesh_id, light_source_pipeline, m_blue_gem),
		create_render_object(m_renderer, "ground", ground_mesh_id, ground_pipeline, m_ground),
		create_render_object(m_renderer, "skybox", skybox_mesh_id, skybox_pipeline),
		create_render_object(m_renderer, "text", m_fps_mesh->GetAssetId(), text_pipeline, m_fps_label),
		create_render_object(m_renderer, "title", m_title_mesh->GetAssetId(), rainbow_text_pipeline, m_title_label)
	};
}

void Scene::OnViewportResized(int width, int height)
{
	m_camera.OnViewportResized(width, height);
	if (m_fps_mesh)
		m_fps_mesh->OnViewportResized(width, height);
	if (m_title_mesh)
		m_title_mesh->OnViewportResized(width, height);
}

void Scene::Update(double delta_time, Input const & input)
{
	const float dt = static_cast<float>(delta_time);
	m_timer += dt;

	m_frame_timer += dt;
	m_frame_count++;
	if (m_frame_timer >= 1.0)
	{
		float fps = static_cast<float>(m_frame_count) / m_frame_timer;
		m_fps_mesh->SetText("FPS: " + std::to_string(static_cast<int>(fps)));
		m_frame_timer = 0.0;
		m_frame_count = 0;
	}

	m_camera.Update(delta_time, input);

	glm::vec3 bg_color;
	bg_color.r = std::sin(m_timer) / 2.0f + 0.5f;
	bg_color.g = std::cos(m_timer) / 2.0f + 0.5f;
	bg_color.b = std::tan(m_timer) / 2.0f + 0.5f;
	m_renderer.SetClearColor(bg_color);

	update_sword_transform(0, m_sword0.m_model, m_timer, dt);
	update_sword_transform(1, m_sword1.m_model, m_timer, dt);
	update_gem_transform(m_red_gem.m_model, dt);
	update_gem_transform(m_green_gem.m_model, dt);
	update_gem_transform(m_blue_gem.m_model, dt);

	glm::mat4 const & red_gem_transform = m_red_gem.m_model;
	m_lights.SetPointLight1(PointLight{
		.m_pos{ red_gem_transform[3][0], red_gem_transform[3][1], red_gem_transform[3][2] },
		.m_color{ 1.0, 0.0, 0.0 },
		.m_radius = 20.0f
		});

	glm::mat4 const & green_gem_transform = m_green_gem.m_model;
	m_lights.SetPointLight2(PointLight{
		.m_pos{ green_gem_transform[3][0], green_gem_transform[3][1], green_gem_transform[3][2] },
		.m_color{ 0.0, 1.0, 0.0 },
		.m_radius = 20.0f
		});

	glm::mat4 const & blue_gem_transform = m_blue_gem.m_model;
	m_lights.SetPointLight3(PointLight{
		.m_pos{ blue_gem_transform[3][0], blue_gem_transform[3][1], blue_gem_transform[3][2] },
		.m_color{ 0.0, 0.0, 1.0 },
		.m_radius = 20.0f
		});

	m_title_label.m_time = m_timer;
}

void Scene::Render() const
{
	m_renderer.Render();
}
