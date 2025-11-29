// TextMesh.ixx

module;

#include <functional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

export module TextMesh;

import AssetId;
import FontAtlas;
import GraphicsApi;
import Mesh;
import Renderer;
import Vertex;

export class TextMesh
{
public:
	using VertexT = Texture2dVertex;

	using UpdateMeshCallbackT = std::function<void(AssetId, Mesh &&)>;

	explicit TextMesh(
		GraphicsApi const & graphics_api,
		std::string const & text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		int viewport_width,
		int viewport_height);

	Mesh CreateMesh();

	void OnViewportResized(int width, int height);

	void SetText(std::string const & text);

	void SetUpdateMeshCallback(UpdateMeshCallbackT const & callback) { m_update_mesh_callback = callback; }

	void SetAssetId(AssetId asset_id) { m_asset_id = asset_id; }
	AssetId GetAssetId() const { return m_asset_id; }

	float GetScreenPxRange() const { return m_screen_px_range; }

private:
	GraphicsApi const & m_graphics_api;
	AssetId m_asset_id;
	UpdateMeshCallbackT m_update_mesh_callback;

	std::string m_text;
	FontAtlas const & m_font_atlas;
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
	float font_size,
	glm::vec2 origin,
	int viewport_width,
	int viewport_height)
	: m_graphics_api(graphics_api)
	, m_text(text)
	, m_font_atlas(font_atlas)
	, m_font_size(font_size)
	, m_screen_px_range(font_size * font_atlas.GetPxRange())
	, m_origin(origin)
	, m_viewport_width(viewport_width)
	, m_viewport_height(viewport_height)
{
}

Mesh TextMesh::CreateMesh()
{
	std::vector<VertexT> verts;
	std::vector<Mesh::IndexT> indices;

	if (m_viewport_width == 0 || m_viewport_height == 0)
		return Mesh{ m_graphics_api, verts, indices };

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
		if (!g.m_plane_bounds.has_value() || !g.m_atlas_bounds.has_value())
		{
			pen.x += g.m_advance * width_scale;
			continue;
		}

		// left, bottom, right, top
		glm::vec4 pb = g.m_plane_bounds.value();
		pb.x *= width_scale;
		pb.z *= width_scale;
		pb.y *= height_scale;
		pb.w *= height_scale;
		glm::vec4 uv = g.m_atlas_bounds.value();
		uv.x /= m_font_atlas.GetWidth();
		uv.z /= m_font_atlas.GetWidth();
		uv.y /= m_font_atlas.GetHeight();
		uv.w /= m_font_atlas.GetHeight();

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

		pen.x += g.m_advance * width_scale;
	}

	// Vulkan screen coordinates are different from OpenGL, the y-axis is -1 at the top instead of the bottom of the screen.
	// We could create a projection matrix that flips the y-axis and pass that into the 2d shaders, but for now we'll do this
	if (m_graphics_api.ShouldFlipScreenY())
	{
		for (VertexT & v : verts)
			v.m_pos.y = -v.m_pos.y;
	}

	return Mesh{ m_graphics_api, verts, indices };
}

void TextMesh::OnViewportResized(int width, int height)
{
	if (width == m_viewport_width && height == m_viewport_height)
		return; // no change

	m_viewport_width = width;
	m_viewport_height = height;

	if (m_update_mesh_callback)
		m_update_mesh_callback(m_asset_id, CreateMesh());
}

void TextMesh::SetText(std::string const & text)
{
	if (m_text == text)
		return; // no change

	m_text = text;

	if (m_update_mesh_callback)
		m_update_mesh_callback(m_asset_id, CreateMesh());
}
