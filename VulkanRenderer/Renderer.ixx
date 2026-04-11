// Renderer.ixx

module;

#include <expected>

#include <glm/vec3.hpp>

export module Renderer;

import GraphicsApi;
import GraphicsError;

export class Renderer
{
public:
	explicit Renderer(GraphicsApi const & graphics_api);

	void BeginDraw() const;
	void EndDraw() const;

	void SetClearColor(glm::vec3 const & color) { m_clear_color = color; }

private:
	GraphicsApi const & m_graphics_api;

	glm::vec3 m_clear_color;
};
