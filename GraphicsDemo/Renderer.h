// Renderer.h

#pragma once

#include <glm/vec3.hpp>

#include "RenderObject.h"

class ShaderProgram;

class Renderer
{
public:
	void Init();
	void Update(double delta_time);
	void Render() const;

	void ResizeViewport(int width, int height);

private:
	glm::vec3 m_bg_color;
	glm::mat4 m_view_transform;
	glm::mat4 m_proj_transform;
	std::shared_ptr<ShaderProgram> m_shader_program;
	RenderObject m_render_object;
};
