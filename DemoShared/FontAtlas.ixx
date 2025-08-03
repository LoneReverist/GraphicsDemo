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

import GraphicsApi;
import StbImage;
import Texture;

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
	FontAtlas(GraphicsApi const & graphics_api, std::filesystem::path const & image_path, std::filesystem::path const & json_path)
	{
		init_texture(graphics_api, image_path);
		init_glyphs(json_path);
	}

	std::optional<std::reference_wrapper<const Glyph>> GetGlyph(std::uint32_t unicode) const
	{
		auto it = m_glyphs.find(unicode);
		if (it != m_glyphs.end())
			return std::cref(it->second);
		return std::nullopt; // glyph not found
	}

	Texture const & GetTexture() const { return *m_texture; }
	std::uint32_t GetWidth() const { return m_width; }
	std::uint32_t GetHeight() const { return m_height; }
	float GetPxRange() const { return m_px_range; }

private:
	void init_texture(GraphicsApi const & graphics_api, std::filesystem::path const & image_path)
	{
		PixelFormat format = PixelFormat::RGB_UNORM;

		StbImage image;
		image.LoadImage(image_path, GetPixelSize(format) /*STBI_rgb*/, true /*flip_vertically*/);
		if (!image.IsValid())
			throw std::runtime_error("Failed to load image for font atlas: " + image_path.string());

		m_width = static_cast<std::uint32_t>(image.GetWidth());
		m_height = static_cast<std::uint32_t>(image.GetHeight());

		m_texture = std::make_unique<Texture>(
			graphics_api,
			ImageData{
				.m_data = image.GetData(),
				.m_format = format,
				.m_width = m_width,
				.m_height = m_height
			}, false /*use_mip_map*/);
	}

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

private:
	std::unique_ptr<Texture> m_texture;
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;

	std::unordered_map<std::uint32_t, const Glyph> m_glyphs;
	float m_px_range = 0.0f;
};
