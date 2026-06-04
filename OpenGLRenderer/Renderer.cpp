// Renderer.cpp

module;

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

module Dreamhearth;

import :Renderer;
import :Texture;

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

	void Renderer::BeginTextureDraw(Texture const & target, glm::vec4 const & clear_color) const
	{
		glGetIntegerv(GL_VIEWPORT, m_previous_viewport.data());

		if (m_texture_framebuffer_id == 0)
			glGenFramebuffers(1, &m_texture_framebuffer_id);

		glBindFramebuffer(GL_FRAMEBUFFER, m_texture_framebuffer_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target.GetType(), target.GetId(), 0 /*level*/);

		GLenum draw_buffer = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &draw_buffer);

		GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
			return;

		glViewport(0, 0, static_cast<int>(target.GetWidth()), static_cast<int>(target.GetHeight()));
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Renderer::EndTextureDraw(Texture const & /*target*/) const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(m_previous_viewport[0], m_previous_viewport[1], m_previous_viewport[2], m_previous_viewport[3]);

		if (m_texture_framebuffer_id != 0)
		{
			glDeleteFramebuffers(1, &m_texture_framebuffer_id);
			m_texture_framebuffer_id = 0;
		}
	}
} // namespace Dreamhearth
