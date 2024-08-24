// Scene.cpp

module;

#include <array>
#include <numbers>

#include <glm/gtc/matrix_transform.hpp>

module Scene;

//import <array>;
import <filesystem>;
//import <numbers>;

import Mesh;

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

	Mesh create_ground_mesh()
	{
		float scale = 30.0f;

		std::vector<TextureVertex> verts {
			{ { -scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0 } },
			{ {  scale,  scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 1.0 } },
			{ { -scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 0.0 } },
			{ {  scale, -scale, 0.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 0.0 } } };

		std::vector<unsigned int> indices{
			1, 0, 2,
			1, 2, 3 };

		return Mesh{ std::move(verts), std::move(indices) };
	}

	Mesh create_skybox_mesh()
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

		std::vector<unsigned int> indices{
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

		return Mesh{ std::move(verts), std::move(indices) };
	}
}

void Scene::Init()
{
	const std::filesystem::path resources_path = std::filesystem::path("..") / "resources";

	const int color_shader_id = m_renderer.LoadShaderProgram(
		resources_path / "shaders" / "color_vs.txt",
		resources_path / "shaders" / "color_fs.txt");
	const int texture_shader_id = m_renderer.LoadShaderProgram(
		resources_path / "shaders" / "texture_vs.txt",
		resources_path / "shaders" / "texture_fs.txt");
	const int light_source_shader_id = m_renderer.LoadShaderProgram(
		resources_path / "shaders" / "light_source_vs.txt",
		resources_path / "shaders" / "light_source_fs.txt");
	const int skybox_shader_id = m_renderer.LoadShaderProgram(
		resources_path / "shaders" / "skybox_vs.txt",
		resources_path / "shaders" / "skybox_fs.txt");

	const int sword_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "skullsword.obj");
	const int red_gem_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "redgem.obj");
	const int green_gem_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "greengem.obj");
	const int blue_gem_mesh_id = m_renderer.LoadMesh(resources_path / "objects" / "bluegem.obj");
	const int ground_mesh_id = m_renderer.AddMesh(std::move(create_ground_mesh()));
	const int skybox_mesh_id = m_renderer.AddMesh(std::move(create_skybox_mesh()));

	const int ground_tex_id = m_renderer.LoadTexture(resources_path / "textures" / "skybox" / "top.jpg");
	const int skybox_tex_id = m_renderer.LoadCubeMap(std::array<std::filesystem::path, 6> {
		resources_path / "textures" / "skybox" / "right.jpg",
		resources_path / "textures" / "skybox" / "left.jpg",
		resources_path / "textures" / "skybox" / "top.jpg",
		resources_path / "textures" / "skybox" / "bottom.jpg",
		resources_path / "textures" / "skybox" / "front.jpg",
		resources_path / "textures" / "skybox" / "back.jpg" });

	m_sword0 = create_object(sword_mesh_id, color_shader_id);
	m_sword1 = create_object(sword_mesh_id, color_shader_id);
	m_red_gem = create_object(red_gem_mesh_id, light_source_shader_id);
	m_green_gem = create_object(green_gem_mesh_id, light_source_shader_id);
	m_blue_gem = create_object(blue_gem_mesh_id, light_source_shader_id);
	m_ground = create_object(ground_mesh_id, texture_shader_id, ground_tex_id);

	m_skybox = std::make_shared<RenderObject>(skybox_mesh_id, skybox_shader_id, skybox_tex_id);
	m_renderer.SetSkybox(m_skybox);

	m_sword0->SetColor({ 0.6, 0.6, 0.6 });
	m_sword1->SetColor({ 0.6, 0.6, 0.6 });
	m_red_gem->SetColor({ 1.0, 0.0, 0.0 });
	m_green_gem->SetColor({ 0.0, 1.0, 0.0 });
	m_blue_gem->SetColor({ 0.0, 0.0, 1.0 });

	init_sword_transform(0, m_sword0->ModifyWorldTransform());
	init_sword_transform(1, m_sword1->ModifyWorldTransform());
	init_gem_transform(0, m_red_gem->ModifyWorldTransform());
	init_gem_transform(1, m_green_gem->ModifyWorldTransform());
	init_gem_transform(2, m_blue_gem->ModifyWorldTransform());

	m_renderer.SetAmbientLightColor(glm::vec3(0.2, 0.2, 0.2));

	m_renderer.SetSpotLight(SpotLight{
		.m_pos{ 0.0f, 0.0f, 25.0f },
		.m_dir{ 0.0f, 0.0f, -1.0f },
		.m_color{ 1.0f, 1.0f, 1.0f },
		.m_inner_radius{ 0.988f },
		.m_outer_radius{ 0.986f } });

	m_camera_pos = glm::vec3{ 0.0f, -10.0f, 5.0f };
	m_camera_dir = glm::normalize(glm::vec3{ 0.0f, 0.0f, 2.5f } - m_camera_pos);
	m_renderer.SetCamera(m_camera_pos, m_camera_dir);
}

void Scene::Update(double delta_time, Input const & input)
{
	const float dt = static_cast<float>(delta_time);
	m_timer += dt;

	update_camera(dt, input);

	glm::vec3 bg_color;
	bg_color.r = std::sin(m_timer) / 2.0f + 0.5f;
	bg_color.g = std::cos(m_timer) / 2.0f + 0.5f;
	bg_color.b = std::tan(m_timer) / 2.0f + 0.5f;
	m_renderer.SetClearColor(bg_color);

	update_sword_transform(0, m_sword0->ModifyWorldTransform(), m_timer, dt);
	update_sword_transform(1, m_sword1->ModifyWorldTransform(), m_timer, dt);
	update_gem_transform(m_red_gem->ModifyWorldTransform(), dt);
	update_gem_transform(m_green_gem->ModifyWorldTransform(), dt);
	update_gem_transform(m_blue_gem->ModifyWorldTransform(), dt);

	glm::mat4 const & red_gem_transform = m_red_gem->GetWorldTransform();
	m_renderer.SetPointLight1(PointLight{
		.m_pos{ red_gem_transform[3][0], red_gem_transform[3][1], red_gem_transform[3][2] },
		.m_color{ 1.0, 0.0, 0.0 },
		.m_radius{ 10.0f } });

	glm::mat4 const & green_gem_transform = m_green_gem->GetWorldTransform();
	m_renderer.SetPointLight2(PointLight{
		.m_pos{ green_gem_transform[3][0], green_gem_transform[3][1], green_gem_transform[3][2] },
		.m_color{ 0.0, 1.0, 0.0 },
		.m_radius{ 10.0f } });

	glm::mat4 const & blue_gem_transform = m_blue_gem->GetWorldTransform();
	m_renderer.SetPointLight3(PointLight{
		.m_pos{ blue_gem_transform[3][0], blue_gem_transform[3][1], blue_gem_transform[3][2] },
		.m_color{ 0.0, 0.0, 1.0 },
		.m_radius{ 10.0f } });
}

std::shared_ptr<RenderObject> Scene::create_object(int mesh_id, int shader_id, int tex_id /*= -1*/) const
{
	auto obj = std::make_shared<RenderObject>(mesh_id, shader_id, tex_id);
	m_renderer.AddRenderObject(obj);
	return obj;
}

void Scene::update_camera(float dt, Input const & input)
{
	glm::vec3 up_dir{ 0.0f, 0.0f, 1.0f };
	glm::vec3 right_dir = glm::cross(m_camera_dir, up_dir);
	glm::vec3 forward_dir = glm::cross(up_dir, right_dir);

	glm::vec3 dir_velocity{ 0.0f, 0.0f, 0.0f };
	if (input.KeyIsPressed(Input::Key::Up))
		dir_velocity += glm::vec3{ 1.0, 0.0, 0.0 };
	if (input.KeyIsPressed(Input::Key::Down))
		dir_velocity += glm::vec3{ -1.0, 0.0, 0.0 };
	if (input.KeyIsPressed(Input::Key::Right))
		dir_velocity += glm::vec3{ 0.0, 0.0, -1.0 };
	if (input.KeyIsPressed(Input::Key::Left))
		dir_velocity += glm::vec3{ 0.0, 0.0, 1.0 };

	bool rotate = glm::length(dir_velocity) > 0.0;
	if (rotate)
	{
		const float rot_speed = std::numbers::pi_v<float>;
		dir_velocity = glm::normalize(dir_velocity);
		m_camera_dir = glm::rotate(glm::mat4(1.0), dir_velocity.x * rot_speed * dt, right_dir) * glm::vec4(m_camera_dir, 0.0);
		m_camera_dir = glm::rotate(glm::mat4(1.0), dir_velocity.z * rot_speed * dt, up_dir) * glm::vec4(m_camera_dir, 0.0);
	}

	glm::vec3 velocity{ 0.0f, 0.0f, 0.0f };
	if (input.KeyIsPressed('W'))
		velocity += forward_dir;
	if (input.KeyIsPressed('S'))
		velocity -= forward_dir;
	if (input.KeyIsPressed('D'))
		velocity += right_dir;
	if (input.KeyIsPressed('A'))
		velocity -= right_dir;

	bool move = glm::length(velocity) > 0.0;
	if (move)
	{
		const float speed = 10.0f;
		m_camera_pos += glm::normalize(velocity) * (speed * dt);
	}

	if (move || rotate)
		m_renderer.SetCamera(m_camera_pos, m_camera_dir);
}
