// FontAtlas.ixx

module;

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <nlohmann/json.hpp>

export module FontAtlas;

import StbImage;

export class FontAtlas
{
public:
	struct Glyph
	{
		std::uint32_t m_unicode = 0;
		float m_advance = 0.0f;
		std::optional<glm::vec4> m_plane_bounds; // left, bottom, right, top
		std::optional<glm::vec4> m_atlas_bounds; // left, bottom, right, top (pixels)
	};

public:
	FontAtlas(std::filesystem::path const & image_path, std::filesystem::path const & json_path)
	{
		m_image.LoadImage(image_path, 3 /*rgb*/);
		if (!m_image.IsValid())
			throw std::runtime_error("Failed to load image for font atlas: " + image_path.string());

		std::ifstream file(json_path);
		if (!file)
			throw std::runtime_error("Failed to open JSON file: " + json_path.string());

		nlohmann::json json;
		file >> json;

		auto atlas = json["atlas"];
		m_px_range = atlas["distanceRange"].get<float>();

		for (const auto & g : json["glyphs"])
		{
			Glyph glyph;
			glyph.m_unicode = g["unicode"].get<std::uint32_t>();
			glyph.m_advance = g["advance"].get<float>();

			auto pb = g.find("planeBounds");
			if (pb != g.end())
				glyph.m_plane_bounds = glm::vec4((*pb)["left"], (*pb)["bottom"], (*pb)["right"], (*pb)["top"]);

			auto ab = g.find("atlasBounds");
			if (ab != g.end())
				glyph.m_atlas_bounds = glm::vec4((*ab)["left"], (*ab)["bottom"], (*ab)["right"], (*ab)["top"]);

			m_glyphs.emplace(glyph.m_unicode, std::move(glyph));
		}
	}

	std::optional<std::reference_wrapper<const Glyph>> GetGlyph(std::uint32_t unicode) const
	{
		auto it = m_glyphs.find(unicode);
		if (it != m_glyphs.end())
			return std::cref(it->second);
		return std::nullopt; // glyph not found
	}

	int GetWidth() const { return m_image.GetWidth(); }
	int GetHeight() const { return m_image.GetHeight(); }

private:
	StbImage m_image;
	std::unordered_map<std::uint32_t, const Glyph> m_glyphs;
	float m_px_range = 0.0f;
};

struct Vertex
{
	glm::vec2 position;
	glm::vec2 texCoord;
};

export std::vector<Vertex> create_text_mesh(const std::string & text, const FontAtlas & atlas, float font_size, glm::vec2 origin)
{
	std::vector<Vertex> vertices;
	glm::vec2 pen = origin;

	for (char c : text)
	{
		auto glyph = atlas.GetGlyph(static_cast<std::uint32_t>(c));
		if (!glyph.has_value())
			continue; // skip missing glyphs

		FontAtlas::Glyph const & g = glyph.value();

		// For the space character we just advance the pen position
		if (!g.m_plane_bounds.has_value() || !g.m_atlas_bounds.has_value())
		{
			pen.x += g.m_advance * font_size;
			continue;
		}

		glm::vec4 pb = g.m_plane_bounds.value() * font_size;
		glm::vec4 ab = g.m_atlas_bounds.value();

		glm::vec2 uv_bl = glm::vec2(ab.x / atlas.GetWidth(), ab.y / atlas.GetHeight());
		glm::vec2 uv_tr = glm::vec2(ab.z / atlas.GetWidth(), ab.w / atlas.GetHeight());

		// 2 triangles (6 vertices)
		vertices.push_back({ {pen.x + pb.x, pen.y + pb.y}, {uv_bl.x, uv_bl.y} }); // bottom-left
		vertices.push_back({ {pen.x + pb.x, pen.y + pb.w}, {uv_bl.x, uv_tr.y} }); // top-left
		vertices.push_back({ {pen.x + pb.z, pen.y + pb.w}, {uv_tr.x, uv_tr.y} }); // top-right

		vertices.push_back({ {pen.x + pb.x, pen.y + pb.y}, {uv_bl.x, uv_bl.y} }); // bottom-left
		vertices.push_back({ {pen.x + pb.z, pen.y + pb.w}, {uv_tr.x, uv_tr.y} }); // top-right
		vertices.push_back({ {pen.x + pb.z, pen.y + pb.y}, {uv_tr.x, uv_bl.y} }); // bottom-right

		pen.x += g.m_advance * font_size;
	}

	return vertices;
}
