// Texture.ixx

module;

#include <array>
#include <cstdint>
#include <expected>

#include <vulkan/vulkan_raii.hpp>

export module Dreamhearth:Texture;

import :RenderContext;
import :GraphicsError;

namespace Dreamhearth
{
	export enum class PixelFormat : std::uint8_t { RGB_UNORM, RGBA_UNORM, RGB_SRGB, RGBA_SRGB };

	export std::uint8_t GetPixelSize(PixelFormat format)
	{
		if (format == PixelFormat::RGBA_UNORM || format == PixelFormat::RGBA_SRGB)
			return 4;
		if (format == PixelFormat::RGB_UNORM || format == PixelFormat::RGB_SRGB)
			return 3;
		return 0;
	}

	export struct ImageData
	{
		std::uint8_t const * data = nullptr;
		PixelFormat format = PixelFormat::RGBA_SRGB;
		std::uint32_t width = 0;
		std::uint32_t height = 0;

		bool IsValid() const;
		std::uint64_t GetSize() const;
	};

	export struct CubeImageData
	{
		std::array<std::uint8_t const *, 6> data;
		PixelFormat format = PixelFormat::RGBA_SRGB;
		std::uint32_t width = 0;
		std::uint32_t height = 0;

		bool IsValid() const;
		std::uint64_t GetSize() const;
	};

	export class Texture
	{
	public:
		Texture() = default;

		Texture(Texture && other) = default;
		Texture & operator=(Texture && other) = default;

		Texture(Texture const &) = delete;
		Texture & operator=(Texture const &) = delete;

		std::expected<void, GraphicsError> Create(RenderContext const & render_context, ImageData const & image_data, bool use_mip_map = true);
		std::expected<void, GraphicsError> Create(RenderContext const & render_context, CubeImageData const & image_data);
		std::expected<void, GraphicsError> CreateRenderTarget(RenderContext const & render_context, std::uint32_t width, std::uint32_t height);

		bool IsValid() const;

		vk::Image GetImage() const { return *m_image; }
		vk::raii::ImageView const & GetImageView() const { return m_image_view; }
		vk::raii::Sampler const & GetSampler() const { return m_sampler; }
		vk::ImageLayout GetLayout() const { return m_layout; }
		void SetLayout(vk::ImageLayout layout) const { m_layout = layout; }

		std::uint32_t GetWidth() const { return m_width; }
		std::uint32_t GetHeight() const { return m_height; }

	private:
		vk::raii::Image m_image = nullptr;
		vk::raii::DeviceMemory m_image_memory = nullptr;
		vk::raii::ImageView m_image_view = nullptr;
		vk::raii::Sampler m_sampler = nullptr;
		mutable vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;
	};
} // namespace Dreamhearth
