// TextMesh.ixx

module;

#include <expected>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

export module TextMesh;

import AssetPool;
import FontAtlas;
import GraphicsApi;
import GraphicsError;
import Mesh;
import MeshManager;
import Renderer;
import Vertex;

export class TextMesh
{
public:
	using VertexT = Texture2dVertex;

	using UpdateMeshCallbackT = std::function<void(AssetId, Mesh)>;

	explicit TextMesh(
		GraphicsApi const & graphics_api,
		std::string const & text,
		FontAtlas const & font_atlas,
		std::uint32_t font_tex_width,
		std::uint32_t font_tex_height,
		float font_size,
		glm::vec2 origin,
		int viewport_width,
		int viewport_height);

	std::expected<Mesh, GraphicsError> CreateMesh() const;

	void OnViewportResized(int width, int height);

	void SetText(std::string const & text);

	void SetUpdateMeshCallback(UpdateMeshCallbackT const & callback) { m_update_mesh_callback = callback; }

	void SetMeshId(MeshId<VertexT> mesh_id) { m_mesh_id = mesh_id; }
	MeshId<VertexT> GetMeshId() const { return m_mesh_id; }

	float GetScreenPxRange() const { return m_screen_px_range; }

private:
	GraphicsApi const & m_graphics_api;
	MeshId<VertexT> m_mesh_id;
	UpdateMeshCallbackT m_update_mesh_callback;

	std::string m_text;
	FontAtlas const & m_font_atlas;
	std::uint32_t m_font_tex_width = 0;
	std::uint32_t m_font_tex_height = 0;
	float m_font_size = 0.0f;
	float m_screen_px_range = 0.0f;
	glm::vec2 m_origin;

	int m_viewport_width = 0;
	int m_viewport_height = 0;
};

TextMesh::TextMesh(
	GraphicsApi const & graphics_api,
	std::string const & text,
	FontAtlas const & font_atlas,
	std::uint32_t font_tex_width,
	std::uint32_t font_tex_height,
	float font_size,
	glm::vec2 origin,
	int viewport_width,
	int viewport_height)
	: m_graphics_api(graphics_api)
	, m_text(text)
	, m_font_atlas(font_atlas)
	, m_font_tex_width(font_tex_width)
	, m_font_tex_height(font_tex_height)
	, m_font_size(font_size)
	, m_screen_px_range(font_size * font_atlas.GetPxRange())
	, m_origin(origin)
	, m_viewport_width(viewport_width)
	, m_viewport_height(viewport_height)
{
}

std::expected<Mesh, GraphicsError> TextMesh::CreateMesh() const
{
	if (m_font_tex_width == 0 || m_font_tex_height == 0
		|| m_viewport_width == 0 || m_viewport_height == 0
		|| m_text.empty())
	{
		return Mesh{ m_graphics_api }; // an empty mesh is an expected result here
	}

	std::vector<VertexT> verts;
	std::vector<Mesh::IndexT> indices;

	// convert font size to screen coordinates -1 to 1
	float height_scale = m_font_size * (2.0f / m_viewport_height);
	float width_scale = m_font_size * (2.0f / m_viewport_width);

	glm::vec2 pen = m_origin;
	for (char c : m_text)
	{
		auto glyph = m_font_atlas.GetGlyph(static_cast<std::uint32_t>(c));
		if (!glyph.has_value())
			continue; // skip missing glyphs

		FontAtlas::Glyph const & g = glyph.value();

		// For the space character we just advance the pen position
		if (!g.plane_bounds.has_value() || !g.atlas_bounds.has_value())
		{
			pen.x += g.advance * width_scale;
			continue;
		}

		// left, bottom, right, top
		glm::vec4 pb = g.plane_bounds.value();
		pb.x *= width_scale;
		pb.z *= width_scale;
		pb.y *= height_scale;
		pb.w *= height_scale;
		glm::vec4 uv = g.atlas_bounds.value();
		uv.x /= m_font_tex_width;
		uv.z /= m_font_tex_width;
		uv.y /= m_font_tex_height;
		uv.w /= m_font_tex_height;

		// 2 triangles
		Mesh::IndexT start_vi = verts.size();
		verts.push_back({ { pen.x + pb.x, pen.y + pb.w }, { uv.x, uv.w } }); // top-left
		verts.push_back({ { pen.x + pb.z, pen.y + pb.w }, { uv.z, uv.w } }); // top-right
		verts.push_back({ { pen.x + pb.x, pen.y + pb.y }, { uv.x, uv.y } }); // bottom-left
		verts.push_back({ { pen.x + pb.z, pen.y + pb.y }, { uv.z, uv.y } }); // bottom-right

		indices.push_back(static_cast<Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<Mesh::IndexT>(start_vi + 0));
		indices.push_back(static_cast<Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<Mesh::IndexT>(start_vi + 1));
		indices.push_back(static_cast<Mesh::IndexT>(start_vi + 2));
		indices.push_back(static_cast<Mesh::IndexT>(start_vi + 3));

		pen.x += g.advance * width_scale;
	}

	// Vulkan screen coordinates are different from OpenGL, the y-axis is -1 at the top instead of the bottom of the screen.
	// We could create a projection matrix that flips the y-axis and pass that into the 2d shaders, but for now we'll do this
	if (m_graphics_api.ShouldFlipScreenY())
	{
		for (VertexT & v : verts)
			v.pos.y = -v.pos.y;
	}

	Mesh mesh{ m_graphics_api };
	std::expected<void, GraphicsError> result = mesh.Create(verts, indices);
	if (!result.has_value())
		return std::unexpected{ result.error().AddToMessage(" TextMesh::CreateMesh: Failed to create mesh.") };

	return mesh;
}

void TextMesh::OnViewportResized(int width, int height)
{
	if (width == m_viewport_width && height == m_viewport_height)
		return; // no change

	m_viewport_width = width;
	m_viewport_height = height;

	if (m_update_mesh_callback)
	{
		std::expected<Mesh, GraphicsError> mesh = CreateMesh();
		if (!mesh.has_value())
		{
			std::cout << "TextMesh::OnViewportResized: Failed to create mesh: " << mesh.error().GetMessage() << std::endl;
			return;
		}

		m_update_mesh_callback(m_mesh_id, std::move(mesh.value()));
	}
}

void TextMesh::SetText(std::string const & text)
{
	if (m_text == text)
		return; // no change

	m_text = text;

	if (m_update_mesh_callback)
	{
		std::expected<Mesh, GraphicsError> mesh = CreateMesh();
		if (!mesh.has_value())
		{
			std::cout << "TextMesh::SetText: Failed to create mesh: " << mesh.error().GetMessage() << std::endl;
			return;
		}

		m_update_mesh_callback(m_mesh_id, std::move(mesh.value()));
	}
}
