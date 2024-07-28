// Renderer.h

#pragma once

#include <glm/vec3.hpp>

#include "RenderObject.h"

class ShaderProgram;

class Renderer
{
public:
	void Init();
	void Update(float time);
	void Render() const;

	void ResizeViewport(int width, int height) const;

private:
	glm::vec3 m_bg_color;
	std::shared_ptr<ShaderProgram> m_shader_program;
	RenderObject m_render_object;
};
