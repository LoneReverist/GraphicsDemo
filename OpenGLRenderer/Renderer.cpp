// Renderer.cpp

module;

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

module Dreamhearth;

import :Renderer;

namespace Dreamhearth
{
	Renderer::Renderer(RenderContext const & render_context)
		: m_render_context(render_context)
	{
	}

	void Renderer::SetViewport(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
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
} // namespace Dreamhearth
