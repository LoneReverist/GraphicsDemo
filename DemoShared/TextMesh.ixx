// TextMesh.ixx

module;

#include <iostream>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

export module TextMesh;

import AssetId;
import FontAtlas;
import Mesh;
import Renderer;
import Vertex;

export class TextMesh
{
public:
	using VertexT = Texture2dVertex;

	static TextMesh Create(
		Renderer & renderer,
		std::string const & text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin,
		int viewport_width,
		int viewport_height)
	{
		TextMesh text_mesh{ renderer, text, font_atlas, font_size, origin };

		Mesh mesh = text_mesh.create_mesh(viewport_width, viewport_height);
		text_mesh.m_asset_id.m_index = renderer.AddMesh(std::move(mesh));
		return text_mesh;
	}

	void OnViewportResized(int width, int height)
	{
		Mesh mesh = create_mesh(width, height);
		m_renderer.UpdateMesh(m_asset_id.m_index, std::move(mesh));
	}

	void SetText(std::string const & text, int viewport_width, int viewport_height)
	{
		if (m_text == text)
			return; // no change
		m_text = text;
		OnViewportResized(viewport_width, viewport_height);
	}

	AssetId<VertexT> GetAssetId() const { return m_asset_id; }

private:
	TextMesh(
		Renderer & renderer,
		std::string const & text,
		FontAtlas const & font_atlas,
		float font_size,
		glm::vec2 origin)
		: m_renderer(renderer)
		, m_text(text)
		, m_font_atlas(font_atlas)
		, m_font_size(font_size)
		, m_origin(origin)
	{
	}

	Mesh create_mesh(int viewport_width, int viewport_height)
	{
		std::vector<VertexT> verts;
		std::vector<Mesh::IndexT> indices;

		if (viewport_height == 0)
			return Mesh{ verts, indices };

		// convert font size to screen coordinates -1 to 1
		float height_scale = m_font_size * (2.0f / viewport_height);
		float width_scale = m_font_size * (2.0f / viewport_width);

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

		return Mesh{ verts, indices };
	}

private:
	Renderer & m_renderer;
	AssetId<VertexT> m_asset_id;

	std::string m_text;
	FontAtlas const & m_font_atlas;
	float m_font_size;
	glm::vec2 m_origin;
};
