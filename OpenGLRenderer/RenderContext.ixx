// RenderContext.ixx

module;

#include <cstdint>

export module Dreamhearth:RenderContext;

namespace Dreamhearth
{
	// For compatibility with Vulkan implementation
	export enum class DrawFrameResult
	{
		Success,
		SwapChainOutOfDate,
		SurfaceLost
	};

	export class RenderContext
	{
	public:
		constexpr static std::uint32_t MaxFramesInFlight = 1;

		using LoadProcFn = void * (char const *);

		explicit RenderContext(
			int width_pixels,
			int height_pixels,
			LoadProcFn * load_proc_fn);
		~RenderContext();


		// For compatibility with Vulkan implementation
		void RecreateSwapChain(int width_pixels, int height_pixels) const {}
		void WaitForLastFrame() const {}
		bool ShouldFlipScreenY() const { return false; } // glm expects opengl style screen coordinates
		std::uint32_t GetCurFrameIndex() const { return 0; }
	};
} // namespace Dreamhearth
