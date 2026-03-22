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
	explicit FontAtlas(GraphicsApi const & graphics_api, std::filesystem::path const & image_path, std::filesystem::path const & json_path)
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

		m_texture = std::make_unique<Texture>(graphics_api);
		std::expected<void, GraphicsError> result = m_texture->Create(
			ImageData{
				.data = image.GetData(),
				.format = format,
				.width = m_width,
				.height = m_height
			},
			false /*use_mip_map*/);
		if (!result.has_value())
			throw std::runtime_error("Failed to create texture for font atlas: " + result.error().GetMessage());
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
	std::unique_ptr<Texture> m_texture;
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;

	std::unordered_map<std::uint32_t, const Glyph> m_glyphs;
	float m_px_range = 0.0f;
};
