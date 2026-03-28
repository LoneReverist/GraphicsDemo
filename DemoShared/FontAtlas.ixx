// FontAtlas.ixx

module;

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include <nlohmann/json.hpp>

export module FontAtlas;

import AssetPool;
import GraphicsApi;
import GraphicsError;
import StbImage;
import Texture;

export class FontAtlas
{
public:
	struct Glyph
	{
		std::uint32_t unicode = 0;
		float advance = 0.0f;
		std::optional<glm::vec4> plane_bounds; // left, bottom, right, top
		std::optional<glm::vec4> atlas_bounds; // left, bottom, right, top (pixels)
	};

public:
	explicit FontAtlas(AssetId texture_id, std::filesystem::path const & json_path)
		: m_texture_id(texture_id)
	{
		init_glyphs(json_path);
	}

	AssetId GetTexture() const { return m_texture_id; }
	float GetPxRange() const { return m_px_range; }

	std::optional<std::reference_wrapper<const Glyph>> GetGlyph(std::uint32_t unicode) const
	{
		auto it = m_glyphs.find(unicode);
		if (it != m_glyphs.end())
			return std::cref(it->second);
		return std::nullopt; // glyph not found
	}

private:
	void init_glyphs(std::filesystem::path const & json_path)
	{
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
			glyph.unicode = g["unicode"].get<std::uint32_t>();
			glyph.advance = g["advance"].get<float>();

			auto pb = g.find("planeBounds");
			if (pb != g.end())
				glyph.plane_bounds = glm::vec4((*pb)["left"], (*pb)["bottom"], (*pb)["right"], (*pb)["top"]);

			auto ab = g.find("atlasBounds");
			if (ab != g.end())
				glyph.atlas_bounds = glm::vec4((*ab)["left"], (*ab)["bottom"], (*ab)["right"], (*ab)["top"]);

			m_glyphs.emplace(glyph.unicode, std::move(glyph));
		}
	}

private:
	AssetId m_texture_id;
	float m_px_range = 0.0f;
	std::unordered_map<std::uint32_t, const Glyph> m_glyphs;
};
