// Renderer.ixx

module;

#include <glm/vec3.hpp>

export module Dreamhearth:Renderer;

import :RenderContext;
import :GraphicsError;

namespace Dreamhearth
{
	export class Renderer
	{
	public:
		explicit Renderer(RenderContext const & render_context);

		void BeginDraw() const;
		void EndDraw() const;

		void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

	private:
		RenderContext const & m_render_context;

		glm::vec3 m_clear_color;
	};
} // namespace Dreamhearth
