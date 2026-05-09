// RenderContext.ixx

module;

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
		using LoadProcFn = void * (char const *);

		explicit RenderContext(
			int width_pixels,
			int height_pixels,
			LoadProcFn * load_proc_fn);
		~RenderContext();

		void SetViewportSize(int width_pixels, int height_pixels) const;

		// For compatibility with Vulkan implementation
		void WaitForLastFrame() const {}
		bool ShouldFlipScreenY() const { return false; } // glm expects opengl style screen coordinates
	};
} // namespace Dreamhearth
