// GraphicsApi.ixx

module;

export module Dreamhearth:GraphicsApi;

namespace Dreamhearth
{
	// For compatibility with Vulkan implementation
	export enum class DrawFrameResult
	{
		Success,
		SwapChainOutOfDate,
		SurfaceLost
	};

	export class GraphicsApi
	{
	public:
		using LoadProcFn = void * (char const *);

		explicit GraphicsApi(
			int width_pixels,
			int height_pixels,
			LoadProcFn * load_proc_fn);
		~GraphicsApi();

		void SetViewportSize(int width_pixels, int height_pixels) const;

		// For compatibility with Vulkan implementation
		void WaitForLastFrame() const {}
		bool ShouldFlipScreenY() const { return false; } // glm expects opengl style screen coordinates
	};
} // namespace Dreamhearth
