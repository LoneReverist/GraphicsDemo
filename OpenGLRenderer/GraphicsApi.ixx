// GraphicsApi.ixx

module;

export module Dreamhearth:GraphicsApi;

namespace Dreamhearth
{
	export class GraphicsApi
	{
	public:
		using LoadProcFn = void * (char const *);

		explicit GraphicsApi(LoadProcFn * load_proc_fn);
		~GraphicsApi();

		void SetViewport(int width_pixels, int height_pixels) const;

		bool ShouldFlipScreenY() const { return false; }
	};
} // namespace Dreamhearth
