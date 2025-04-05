// Scene.cpp

module;

#include <array>
#include <filesystem>
#include <numbers>

#include <glm/gtc/matrix_transform.hpp>

module Scene;

import GraphicsApi;
import GraphicsPipeline;
import Mesh;
import PipelineBuilder;
import Texture;
import Vertex;

namespace
{
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

	Mesh create_ground_mesh(GraphicsApi const & graphics_api)
	{
		float scale = 30.0f;

		std::vector<TextureVertex> verts {
			{ { -scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0 } },
			{ {  scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 1.0 } },
			{ { -scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0 } },
			{ {  scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0 } } };
		 
		//std::vector<ColorVertex> verts{
		//	{ { -scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0 } },
		//	{ {  scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0 } },
		//	{ { -scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		//	{ {  scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.5, 0.5, 0.5 } } };

		std::vector<Mesh::index_t> indices{
			1, 0, 2,
			1, 2, 3 };

		return Mesh{ graphics_api, std::move(verts), std::move(indices) };
	}

	Mesh create_skybox_mesh(GraphicsApi const & graphics_api)
	{
		std::vector<PositionVertex> verts {
			{ { -1.0f,  1.0f, -1.0f } },
			{ { -1.0f, -1.0f, -1.0f } },
			{ {  1.0f, -1.0f, -1.0f } },
			{ {  1.0f,  1.0f, -1.0f } },
			{ { -1.0f,  1.0f,  1.0f } },
			{ { -1.0f, -1.0f,  1.0f } },
			{ {  1.0f, -1.0f,  1.0f } },
			{ {  1.0f,  1.0f,  1.0f } } };

		std::vector<Mesh::index_t> indices{
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

		return Mesh{ graphics_api, std::move(verts), std::move(indices) };
	}

	std::unique_ptr<GraphicsPipeline> create_texture_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path,
		Texture const & texture)
	{
		struct VSPushConstant
		{
			alignas(16) glm::mat4 model;
		};
		struct ViewProjUniform
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};
		struct LightsUniform
		{
			alignas(16) glm::vec3 m_ambient_light_color;
			PointLight m_pointlight_1;
			PointLight m_pointlight_2;
			PointLight m_pointlight_3;
			SpotLight m_spotlight;
		};

		PipelineBuilder builder{ scene.GetGraphicsApi() };
		builder.LoadShaders(
			shaders_path / "texture_vert.spv",
			shaders_path / "texture_frag.spv");
		builder.SetVertexType<TextureVertex>();
		builder.SetPushConstantTypes<VSPushConstant, std::nullopt_t>();
		builder.SetVSUniformTypes<ViewProjUniform>();
		builder.SetFSUniformTypes<LightsUniform>();
		builder.SetTexture(texture);

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform(0 /*binding*/,
					ViewProjUniform{
						.view = scene.GetCamera().GetViewTransform(),
						.proj = scene.GetCamera().GetProjTransform()
					});
				pipeline.SetUniform(1 /*binding*/,
					LightsUniform{
						.m_ambient_light_color = scene.GetAmbientLightColor(),
						.m_pointlight_1 = scene.GetPointLight1(),
						.m_pointlight_2 = scene.GetPointLight2(),
						.m_pointlight_3 = scene.GetPointLight3(),
						.m_spotlight = scene.GetSpotLight()
					});
			});
		builder.SetPerObjectConstantsCallback(
			[](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetPushConstants(
					VSPushConstant{
						.model = obj.GetModelTransform()
					},
					std::nullopt);
			});

		return builder.CreatePipeline();
	}

	std::unique_ptr<GraphicsPipeline> create_reflection_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path,
		Texture const & texture)
	{
		struct VSPushConstant
		{
			alignas(16) glm::mat4 model;
		};
		struct ViewProjUniform
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};
		struct LightsUniform
		{
			alignas(16) glm::vec3 m_ambient_light_color;
			PointLight m_pointlight_1;
			PointLight m_pointlight_2;
			PointLight m_pointlight_3;
			SpotLight m_spotlight;
		};
		struct CameraUniform
		{
			alignas(16) glm::vec3 camera_pos_world;
		};

		PipelineBuilder builder{ scene.GetGraphicsApi() };
		builder.LoadShaders(
			shaders_path / "reflection_vert.spv",
			shaders_path / "reflection_frag.spv");
		builder.SetVertexType<NormalVertex>();
		builder.SetPushConstantTypes<VSPushConstant, std::nullopt_t>();
		builder.SetVSUniformTypes<ViewProjUniform>();
		builder.SetFSUniformTypes<LightsUniform, CameraUniform>();
		builder.SetTexture(texture);

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform(0 /*binding*/,
					ViewProjUniform{
						.view = scene.GetCamera().GetViewTransform(),
						.proj = scene.GetCamera().GetProjTransform()
					});
				pipeline.SetUniform(1 /*binding*/,
					LightsUniform{
						.m_ambient_light_color = scene.GetAmbientLightColor(),
						.m_pointlight_1 = scene.GetPointLight1(),
						.m_pointlight_2 = scene.GetPointLight2(),
						.m_pointlight_3 = scene.GetPointLight3(),
						.m_spotlight = scene.GetSpotLight()
					});
				pipeline.SetUniform(2 /*binding*/,
					CameraUniform{
						.camera_pos_world = scene.GetCamera().GetPos()
					});
			});
		builder.SetPerObjectConstantsCallback(
			[](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetPushConstants(
					VSPushConstant{
						.model = obj.GetModelTransform()
					},
					std::nullopt);
			});

		return builder.CreatePipeline();
	}

	std::unique_ptr<GraphicsPipeline> create_skybox_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path,
		Texture const & skybox)
	{
		struct ViewProjUniform
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		PipelineBuilder builder{ scene.GetGraphicsApi() };
		builder.LoadShaders(
			shaders_path / "skybox_vert.spv",
			shaders_path / "skybox_frag.spv");
		builder.SetVertexType<PositionVertex>();
		builder.SetVSUniformTypes<ViewProjUniform>();
		builder.SetTexture(skybox);
		builder.SetDepthTestOptions(DepthTestOptions{
			.m_enable_depth_test = true,
			.m_enable_depth_write = false,
			.m_depth_compare_op = DepthCompareOp::EQUAL
			});

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform(0 /*binding*/,
					ViewProjUniform{
						.view = scene.GetCamera().GetViewTransform(),
						.proj = scene.GetCamera().GetProjTransform()
					});
			});

		return builder.CreatePipeline();
	}

	std::unique_ptr<GraphicsPipeline> create_color_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path)
	{
		struct VSPushConstant
		{
			alignas(16) glm::mat4 model;
		};
		struct ViewProjUniform
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};
		struct LightsUniform
		{
			alignas(16) glm::vec3 m_ambient_light_color;
			PointLight m_pointlight_1;
			PointLight m_pointlight_2;
			PointLight m_pointlight_3;
			SpotLight m_spotlight;
		};

		PipelineBuilder builder{ scene.GetGraphicsApi() };
		builder.LoadShaders(
			shaders_path / "color_vert.spv",
			shaders_path / "color_frag.spv");
		builder.SetVertexType<ColorVertex>();
		builder.SetPushConstantTypes<VSPushConstant, std::nullopt_t>();
		builder.SetVSUniformTypes<ViewProjUniform>();
		builder.SetFSUniformTypes<LightsUniform>();

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform(0 /*binding*/,
					ViewProjUniform{
						.view = scene.GetCamera().GetViewTransform(),
						.proj = scene.GetCamera().GetProjTransform()
					});
				pipeline.SetUniform(1 /*binding*/,
					LightsUniform{
						.m_ambient_light_color = scene.GetAmbientLightColor(),
						.m_pointlight_1 = scene.GetPointLight1(),
						.m_pointlight_2 = scene.GetPointLight2(),
						.m_pointlight_3 = scene.GetPointLight3(),
						.m_spotlight = scene.GetSpotLight()
					});
			});
		builder.SetPerObjectConstantsCallback(
			[](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetPushConstants(
					VSPushConstant{
						.model = obj.GetModelTransform()
					},
					std::nullopt);
			});

		return builder.CreatePipeline();
	}

	std::unique_ptr<GraphicsPipeline> create_light_source_pipeline(
		Scene const & scene,
		std::filesystem::path const & shaders_path)
	{
		struct VSPushConstant
		{
			alignas(16) glm::mat4 model;
		};
		struct FSPushConstant
		{
			alignas(16) glm::vec3 color;
		};
		struct ViewProjUniform
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};
		struct CameraPosUniform
		{
			alignas(16) glm::vec3 camera_pos_world;
		};

		PipelineBuilder builder{ scene.GetGraphicsApi() };
		builder.LoadShaders(
			shaders_path / "light_source_vert.spv",
			shaders_path / "light_source_frag.spv");
		builder.SetVertexType<NormalVertex>();
		builder.SetPushConstantTypes<VSPushConstant, FSPushConstant>();
		builder.SetVSUniformTypes<ViewProjUniform>();
		builder.SetFSUniformTypes<CameraPosUniform>();

		builder.SetPerFrameConstantsCallback(
			[&scene](GraphicsPipeline const & pipeline)
			{
				pipeline.SetUniform(0 /*binding*/,
					ViewProjUniform{
						.view = scene.GetCamera().GetViewTransform(),
						.proj = scene.GetCamera().GetProjTransform()
					});
				pipeline.SetUniform(1 /*binding*/,
					CameraPosUniform{
						.camera_pos_world = scene.GetCamera().GetPos()
					});
			});
		builder.SetPerObjectConstantsCallback(
			[](GraphicsPipeline const & pipeline, RenderObject const & obj)
			{
				pipeline.SetPushConstants(
					VSPushConstant{
						.model = obj.GetModelTransform()
					},
					FSPushConstant{
						.color = obj.GetColor(),
					});
			});

		return builder.CreatePipeline();
	}
}

void Scene::Init()
{
	const std::filesystem::path resources_path = std::filesystem::path("..") / "resources";
	const std::filesystem::path shaders_path = "shaders";

	m_ground_tex = std::make_unique<Texture>(m_graphics_api,
		resources_path / "textures" / "skybox" / "top.jpg");
	m_skybox_tex = std::make_unique<Texture>(m_graphics_api, std::array<std::filesystem::path, 6>{
		resources_path / "textures" / "skybox" / "right.jpg",
		resources_path / "textures" / "skybox" / "left.jpg",
		resources_path / "textures" / "skybox" / "top.jpg",
		resources_path / "textures" / "skybox" / "bottom.jpg",
		resources_path / "textures" / "skybox" / "front.jpg",
		resources_path / "textures" / "skybox" / "back.jpg"
	});

	//const int color_shader_id = m_renderer.AddGraphicsPipeline(
	//	std::move(create_color_pipeline(*this, shaders_path)));
	const int texture_shader_id = m_renderer.AddGraphicsPipeline(
		std::move(create_texture_pipeline(*this, shaders_path, *m_ground_tex)));
	const int light_source_pipeline_id = m_renderer.AddGraphicsPipeline(
		std::move(create_light_source_pipeline(*this, shaders_path)));
	const int reflection_shader_id = m_renderer.AddGraphicsPipeline(
		std::move(create_reflection_pipeline(*this, shaders_path, *m_skybox_tex)));
	const int skybox_pipeline_id = m_renderer.AddGraphicsPipeline(
		std::move(create_skybox_pipeline(*this, shaders_path, *m_skybox_tex)));

	const int sword_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "skullsword.obj");
	const int red_gem_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "redgem.obj");
	const int green_gem_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "greengem.obj");
	const int blue_gem_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "bluegem.obj");
	const int ground_mesh_id = m_renderer.AddMesh(std::move(create_ground_mesh(m_graphics_api)));
	const int skybox_mesh_id = m_renderer.AddMesh(std::move(create_skybox_mesh(m_graphics_api)));

	m_sword0 = m_renderer.CreateRenderObject(sword_mesh_id, reflection_shader_id);
	m_sword1 = m_renderer.CreateRenderObject(sword_mesh_id, reflection_shader_id);
	m_red_gem = m_renderer.CreateRenderObject(red_gem_mesh_id, light_source_pipeline_id);
	m_green_gem = m_renderer.CreateRenderObject(green_gem_mesh_id, light_source_pipeline_id);
	m_blue_gem = m_renderer.CreateRenderObject(blue_gem_mesh_id, light_source_pipeline_id);
	//m_ground = m_renderer.CreateRenderObject(ground_mesh_id, color_shader_id);
	m_ground = m_renderer.CreateRenderObject(ground_mesh_id, texture_shader_id);
	m_skybox = m_renderer.CreateRenderObject(skybox_mesh_id, skybox_pipeline_id);

	//m_sword0->SetColor({ 0.6, 0.6, 0.6 });
	//m_sword1->SetColor({ 0.6, 0.6, 0.6 });
	m_red_gem->SetColor({ 1.0, 0.0, 0.0 });
	m_green_gem->SetColor({ 0.0, 1.0, 0.0 });
	m_blue_gem->SetColor({ 0.0, 0.0, 1.0 });

	init_sword_transform(0, m_sword0->ModifyModelTransform());
	init_sword_transform(1, m_sword1->ModifyModelTransform());
	init_gem_transform(0, m_red_gem->ModifyModelTransform());
	init_gem_transform(1, m_green_gem->ModifyModelTransform());
	init_gem_transform(2, m_blue_gem->ModifyModelTransform());

	m_ambient_light_color = glm::vec3(0.5, 0.5, 0.5);

	m_spotlight = SpotLight{
		.m_pos{ 0.0f, 0.0f, 25.0f },
		.m_dir{ 0.0f, 0.0f, -1.0f },
		.m_color{ 1.0f, 1.0f, 1.0f },
		.m_inner_radius{ 0.988f },
		.m_outer_radius{ 0.986f }
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
		.m_radius{ 20.0f } };

	glm::mat4 const & green_gem_transform = m_green_gem->GetModelTransform();
	m_pointlight_2 = PointLight{
		.m_pos{ green_gem_transform[3][0], green_gem_transform[3][1], green_gem_transform[3][2] },
		.m_color{ 0.0, 1.0, 0.0 },
		.m_radius{ 20.0f } };

	glm::mat4 const & blue_gem_transform = m_blue_gem->GetModelTransform();
	m_pointlight_3 = PointLight{
		.m_pos{ blue_gem_transform[3][0], blue_gem_transform[3][1], blue_gem_transform[3][2] },
		.m_color{ 0.0, 0.0, 1.0 },
		.m_radius{ 20.0f } };
}

void Scene::Render() const
{
	m_renderer.Render();
}
