// RenderContext.ixx

module;

#include <cstdint>
#include <expected>
#include <memory>

export module Dreamhearth:RenderContext;

import :GraphicsError;

namespace Dreamhearth
{
	// For compatibility with Vulkan implementation
	export enum class DrawFrameResult
	{
		Success,
		SwapChainOutOfDate,
		SurfaceLost
	};

	export struct RenderExtent
	{
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};

	export class RenderContext
	{
	public:
		constexpr static std::uint32_t MaxFramesInFlight = 1;

		using LoadProcFn = void * (char const *);

		static std::expected<RenderContext, GraphicsError> Create(
			int width_pixels,
			int height_pixels,
			LoadProcFn * load_proc_fn,
			GraphicsDiagnosticFn on_diagnostic = {});
		~RenderContext();

		RenderContext(RenderContext &&) noexcept = default;
		RenderContext & operator=(RenderContext &&) noexcept = delete;
		RenderContext(RenderContext const &) = delete;
		RenderContext & operator=(RenderContext const &) = delete;

		// For compatibility with Vulkan implementation
		std::expected<void, GraphicsError> RecreateSwapChain(int width_pixels, int height_pixels)
		{
			m_swap_chain_extent = {
				width_pixels > 0 ? static_cast<std::uint32_t>(width_pixels) : 0,
				height_pixels > 0 ? static_cast<std::uint32_t>(height_pixels) : 0
			};
			return {};
		}
		RenderExtent GetSwapChainExtent() const { return m_swap_chain_extent; }
		void WaitForLastFrame() const {}
		bool ShouldFlipScreenY() const { return false; } // glm expects opengl style screen coordinates
		std::uint32_t GetCurFrameIndex() const { return 0; }
		void ReportDiagnostic(GraphicsDiagnostic diagnostic) const noexcept;

	private:
		explicit RenderContext(int width_pixels, int height_pixels, GraphicsDiagnosticFn on_diagnostic);

		std::shared_ptr<GraphicsDiagnosticFn> m_on_diagnostic;
		RenderExtent m_swap_chain_extent;
	};
} // namespace Dreamhearth
