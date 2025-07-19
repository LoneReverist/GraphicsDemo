// Camera.ixx

module;

#include <numbers>

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Camera;

import Input;

export class Camera
{
public:
	Camera(bool flip_proj_y = false) : m_flip_proj_y(flip_proj_y) {}

	void Init(glm::vec3 const & pos, glm::vec3 const & dir);
	void OnViewportResized(int width, int height);

	void Update(double delta_time, Input const & input);

	glm::vec3 const & GetPos() const { return m_pos; }
	glm::vec3 const & GetDir() const { return m_dir; }
	glm::mat4 const & GetViewTransform() const { return m_view_transform; }
	glm::mat4 const & GetProjTransform() const { return m_proj_transform; }

private:
	glm::vec3 m_pos{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_dir{ 0.0f, 0.0f, -1.0f };
	glm::mat4 m_view_transform = 1.0f;
	glm::mat4 m_proj_transform = 1.0f;

	bool m_flip_proj_y = false; // whether to flip the y-axis in the projection matrix

	static constexpr glm::vec3 m_up_dir{ 0.0f, 0.0f, 1.0f }; // a little atypical, but i prefer Z to be up
	static constexpr float m_fov = glm::radians(45.0f);
	static constexpr float m_near_plane = 0.1f;
	static constexpr float m_far_plane = 100.0f;
};

void Camera::Init(glm::vec3 const & pos, glm::vec3 const & dir)
{
	m_pos = pos;
	m_dir = dir;
	m_view_transform = glm::lookAt(m_pos, m_pos + m_dir, m_up_dir);
}

void Camera::OnViewportResized(int width, int height)
{
	if (height == 0)
		return;

	const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	m_proj_transform = glm::perspective(m_fov, aspect_ratio, m_near_plane, m_far_plane);

	if (m_flip_proj_y)
		m_proj_transform[1][1] *= -1; // account for vulkan having flipped y-axis compared to opengl
}

void Camera::Update(double delta_time, Input const & input)
{
	const float dt = static_cast<float>(delta_time);

	glm::vec3 right_dir = glm::cross(m_dir, m_up_dir);
	glm::vec3 forward_dir = glm::cross(m_up_dir, right_dir);

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
		m_dir = glm::rotate(glm::mat4(1.0), dir_velocity.x * rot_speed * dt, right_dir) * glm::vec4(m_dir, 0.0);
		m_dir = glm::rotate(glm::mat4(1.0), dir_velocity.z * rot_speed * dt, m_up_dir) * glm::vec4(m_dir, 0.0);
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
		m_pos += glm::normalize(velocity) * (speed * dt);
	}

	if (move || rotate)
		m_view_transform = glm::lookAt(m_pos, m_pos + m_dir, glm::vec3(0.0, 0.0, 1.0));
}
