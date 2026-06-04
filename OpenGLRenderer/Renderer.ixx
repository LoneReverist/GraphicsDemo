// Renderer.ixx

module;

#include <array>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

export module Dreamhearth:Renderer;

import :RenderContext;
import :GraphicsError;
import :Texture;

namespace Dreamhearth
{
	export class Renderer
	{
	public:
		explicit Renderer(RenderContext const & render_context);

		void SetViewport(int x, int y, int width, int height);
		void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

		void BeginDraw() const;
		void EndDraw() const;
		void BeginTextureDraw(Texture const & target, glm::vec4 const & clear_color) const;
		void EndTextureDraw(Texture const & target) const;

	private:
		RenderContext const & m_render_context;

		glm::vec3 m_clear_color;
		mutable unsigned int m_texture_framebuffer_id = 0;
		mutable std::array<int, 4> m_previous_viewport{ 0, 0, 0, 0 };
	};
} // namespace Dreamhearth
