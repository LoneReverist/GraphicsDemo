// GraphicsApi.ixx

module;

export module GraphicsApi;

export class GraphicsApi
{
public:
	using LoadProcFn = void * (char const *);

	explicit GraphicsApi(LoadProcFn * load_proc_fn);
	~GraphicsApi();

	void SetViewport(int width_pixels, int height_pixels) const;

	bool ShouldFlipScreenY() const { return false; }
};
