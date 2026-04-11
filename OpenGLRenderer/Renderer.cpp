// Renderer.cpp

module;

#include <expected>
#include <memory>
#include <string>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

module Renderer;

Renderer::Renderer(GraphicsApi const & graphics_api)
	: m_graphics_api(graphics_api)
{
}

void Renderer::BeginDraw() const
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndDraw() const
{
}
