// GraphicsApi.ixx

module;

#include <string>

export module GraphicsApi;

export class GraphicsApi
{
public:
	using LoadProcFn = void * (char const *);

	GraphicsApi(LoadProcFn * load_proc_fn);
	~GraphicsApi();

	void SetViewport(int width, int height) const;

	bool ShouldFlipScreenY() const { return false; }
};
