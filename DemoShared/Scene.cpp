// Scene.cpp

module;

#include <algorithm>
#include <array>
#include <expected>
#include <filesystem>
#include <iostream>
#include <numbers>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

module Scene;

import AssetId;
import GraphicsError;
import GraphicsPipeline;
import Mesh;
import PlatformUtils;
import StbImage;
import TextMesh;
import AssimpLoader;

std::unique_ptr<Texture> create_texture(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & filepath)
{
	auto texture = std::make_unique<Texture>(graphics_api);

	PixelFormat format = PixelFormat::RGBA_SRGB;

	StbImage image(filepath, GetPixelSize(format) /*STBI_rgb_alpha*/);
	if (!image.IsValid())
	{
		std::cout << "Failed to load image: " << filepath << std::endl;
		return texture;
	}

	std::expected<void, GraphicsError> result = texture->Create(
		ImageData{
			.m_data = image.GetData(),
			.m_format = format,
			.m_width = static_cast<std::uint32_t>(image.GetWidth()),
			.m_height = static_cast<std::uint32_t>(image.GetHeight())
		});
	if (!result.has_value())
		std::cout << "Failed to create texture from image: " << filepath << std::endl;

	return texture;
}

std::unique_ptr<Texture> create_cubemap_texture(
	GraphicsApi const & graphics_api,
	std::array<std::filesystem::path, 6> const & filepaths)
{
	auto texture = std::make_unique<Texture>(graphics_api);

	PixelFormat format = PixelFormat::RGBA_SRGB;

	int width = 0, height = 0;
	std::array<StbImage, 6> images;
	for (size_t i = 0; i < filepaths.size(); ++i)
	{
		images[i].LoadImage(filepaths[i], GetPixelSize(format) /*STBI_rgb_alpha*/);
		if (!images[i].IsValid())
		{
			std::cout << "Failed to load cubemap image: " << filepaths[i] << std::endl;
			return texture;
		}

		if (i == 0)
		{
			width = images[i].GetWidth();
			height = images[i].GetHeight();
		}
		else if (images[i].GetWidth() != width || images[i].GetHeight() != height)
		{
			std::cout << "Cubemap images must have the same dimensions." << std::endl;
			return texture;
		}
	}

	std::array<std::uint8_t const *, 6> data;
	std::ranges::transform(images, data.begin(),
		[](StbImage const & img) { return img.GetData(); });

	std::expected<void, GraphicsError> result = texture->Create(
		CubeImageData{
			.m_data = data,
			.m_format = format,
			.m_width = static_cast<std::uint32_t>(width),
			.m_height = static_cast<std::uint32_t>(height)
		});
	if (!result.has_value())
		std::cout << "Failed to create cubemap texture." << std::endl;

	return texture;
}

MeshAsset<PositionVertex> Scene::create_skybox_mesh()
{
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

	return create_mesh<PositionVertex>(verts, indices);
}

MeshAsset<TextureVertex> Scene::create_ground_mesh()
{
	float scale = 30.0f;

	std::vector<TextureVertex> verts{
		{ { -scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0 } },
		{ {  scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 1.0 } },
		{ { -scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0 } },
		{ {  scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0 } } };

	//std::vector<ColorVertex> verts{
	//	{ { -scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 } },
	//	{ {  scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0 } },
	//	{ { -scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	//	{ {  scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.5, 0.5, 0.5 } } };

	std::vector<Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return create_mesh<TextureVertex>(verts, indices);
}

std::vector<MeshAsset<ColorVertex>> Scene::create_tree_with_material_meshes()
{
	std::vector<Mesh> tree_meshes = AssimpLoader::LoadObjWithColorMaterial(m_graphics_api, m_resources_path / "objects" / "tree_with_material.obj");
	if (tree_meshes.empty())
	{
		std::cout << "Failed to load tree_with_material.obj with Assimp" << std::endl;
		return {};
	}

	std::vector<MeshAsset<ColorVertex>> tree_with_material_meshes;
	for (auto & mesh : tree_meshes)
	{
		std::expected<int, GraphicsError> mesh_id = m_renderer.AddMesh(std::move(mesh));
		if (!mesh_id.has_value())
		{
			std::cout << "Failed to add tree_with_material mesh to renderer: " << mesh_id.error().GetMessage() << std::endl;
			continue;
		}

		tree_with_material_meshes.push_back(MeshAsset<ColorVertex>{ AssetId{ mesh_id.value() } });
	}
	return tree_with_material_meshes;
}

std::unique_ptr<TextMesh> Scene::create_text_mesh(
	std::string const & text,
	FontAtlas const & font_atlas,
	float font_size,
	glm::vec2 origin,
	int viewport_width,
	int viewport_height)
{
	auto text_mesh = std::make_unique<TextMesh>(m_graphics_api, text, font_atlas, font_size, origin, viewport_width, viewport_height);
	text_mesh->SetUpdateMeshCallback([&renderer = m_renderer](AssetId id, Mesh mesh)
		{
			if (!id.IsValid())
			{
				std::cout << "Scene::create_text_mesh: Invalid AssetId for updating mesh" << std::endl;
				return;
			}

			std::expected<void, GraphicsError> result = renderer.UpdateMesh(id.m_index, std::move(mesh));
			if (!result.has_value())
			{
				std::cout << "Scene::create_text_mesh: Failed to update mesh with id: "
					<< id.m_index << " Error: " << result.error().GetMessage();
			}
		});

	std::expected<int, GraphicsError> mesh_id
		= text_mesh->CreateMesh()
		.and_then([&renderer = m_renderer](Mesh mesh) -> std::expected<int, GraphicsError>
			{
				return renderer.AddMesh(std::move(mesh));
			});

	if (!mesh_id.has_value())
	{
		std::cout << "Scene::create_text_mesh: Failed to create mesh. Error: "
			<< mesh_id.error().GetMessage() << std::endl;
		return text_mesh;
	}

	text_mesh->SetAssetId(AssetId{ mesh_id.value() });
	return text_mesh;
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

Scene::Scene(GraphicsApi const & graphics_api, std::string const & title)
	: m_graphics_api{ graphics_api }
	, m_resources_path{ PlatformUtils::GetExecutableDir() / "resources" }
	, m_title{ title }
	, m_renderer{ graphics_api }
	, m_camera{ graphics_api.ShouldFlipScreenY() }
{
	m_dpi_scale = PlatformUtils::GetDPIScalingFactor();
	float label_font_size = 18.0f * m_dpi_scale;
	float title_font_size = 32.0f * m_dpi_scale;

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

	ColorPipeline color_pipeline = create_pipeline<ColorPipeline>(m_camera, m_lights);
	TexturePipeline ground_pipeline = create_pipeline<TexturePipeline>(m_camera, m_lights, *m_ground_tex);
	SkyboxPipeline skybox_pipeline = create_pipeline<SkyboxPipeline>(m_camera, *m_skybox_tex);
	LightSourcePipeline light_source_pipeline = create_pipeline<LightSourcePipeline>(m_camera);
	ReflectionPipeline reflection_pipeline = create_pipeline<ReflectionPipeline>(m_camera, m_lights, *m_skybox_tex);
	TextPipeline text_pipeline = create_pipeline<TextPipeline>(*m_arial_font);
	RainbowTextPipeline rainbow_text_pipeline = create_pipeline<RainbowTextPipeline>(*m_arial_font);

	MeshAsset<NormalVertex> sword_mesh = create_mesh<NormalVertex>(objects_path / "skullsword.obj");
	init_sword_transform(0, m_sword0.m_model);
	init_sword_transform(1, m_sword1.m_model);
	create_render_object("sword0", sword_mesh, reflection_pipeline, m_sword0);
	create_render_object("sword1", sword_mesh, reflection_pipeline, m_sword1);

	MeshAsset<NormalVertex> red_gem_mesh = create_mesh<NormalVertex>(objects_path / "redgem.obj");
	MeshAsset<NormalVertex> green_gem_mesh = create_mesh<NormalVertex>(objects_path / "greengem.obj");
	MeshAsset<NormalVertex> blue_gem_mesh = create_mesh<NormalVertex>(objects_path / "bluegem.obj");
	m_red_gem.m_color = glm::vec3{ 1.0f, 0.0f, 0.0f };
	m_green_gem.m_color = glm::vec3{ 0.0f, 1.0f, 0.0f };
	m_blue_gem.m_color = glm::vec3{ 0.0f, 0.0f, 1.0f };
	init_gem_transform(0, m_red_gem.m_model);
	init_gem_transform(1, m_green_gem.m_model);
	init_gem_transform(2, m_blue_gem.m_model);
	create_render_object("red gem", red_gem_mesh, light_source_pipeline, m_red_gem);
	create_render_object("green gem", green_gem_mesh, light_source_pipeline, m_green_gem);
	create_render_object("blue gem", blue_gem_mesh, light_source_pipeline, m_blue_gem);

	MeshAsset<TextureVertex> ground_mesh = create_ground_mesh();
	create_render_object("ground", ground_mesh, ground_pipeline, m_ground);

	MeshAsset<PositionVertex> skybox_mesh = create_skybox_mesh();
	create_render_object("skybox", skybox_mesh, skybox_pipeline);

	MeshAsset<NormalVertex> tree_mesh = create_mesh<NormalVertex>(objects_path / "tree.obj");
	m_tree.m_color = glm::vec3{ 0.0f, 0.5f, 0.0f };
	m_tree.m_model = glm::scale(m_tree.m_model, glm::vec3(3.281, 3.281, 3.281)); // meters to feet
	m_tree.m_model = glm::translate(m_tree.m_model, glm::vec3(3.0f, 5.0f, 0.0f));
	create_render_object("tree", tree_mesh, light_source_pipeline, m_tree);

	std::vector<MeshAsset<ColorVertex>> tree_with_material_meshes = create_tree_with_material_meshes();
	m_tree_with_material.m_model = glm::scale(m_tree_with_material.m_model, glm::vec3(3.281, 3.281, 3.281)); // meters to feet
	m_tree_with_material.m_model = glm::translate(m_tree_with_material.m_model, glm::vec3(-3.0f, 5.0f, 0.0f));
	for (auto const & mesh : tree_with_material_meshes)
		create_render_object("tree_with_material", mesh, color_pipeline, m_tree_with_material);

	m_fps_mesh = create_text_mesh("FPS: ", *m_arial_font, label_font_size, glm::vec2{ -0.9, -0.9 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/);
	m_fps_label = TextPipeline::ObjectData{
		.m_screen_px_range = m_fps_mesh->GetScreenPxRange(),
		.m_bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.m_text_color = { 1.0f, 1.0f, 0.0f, 1.0 },
	};
	create_render_object("fps label", *m_fps_mesh, text_pipeline, m_fps_label);

	m_title_mesh = create_text_mesh(m_title, *m_arial_font, title_font_size, glm::vec2{ -0.9, 0.8 } /*origin*/,
		0 /*viewport_width*/, 0 /*viewport_height*/);
	m_title_label = RainbowTextPipeline::ObjectData{
		.m_bg_color = { 0.0f, 0.0f, 0.0f, 0.0f },
		.m_screen_px_range = m_title_mesh->GetScreenPxRange(),
		.m_rainbow_width = 200.0f * m_dpi_scale,
		.m_slant_factor = -1.0f
	};
	create_render_object("title", *m_title_mesh, rainbow_text_pipeline, m_title_label);

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
	std::expected<void, GraphicsError> result = m_renderer.Render();
	if (!result.has_value())
	{
		std::cout << "Scene::Render: Failed to render scene. Error: "
			<< result.error().GetMessage() << std::endl;
	}
}
