// GraphicsApi.ixx

module;

export module GraphicsApi;

import <string>;

export class GraphicsApi
{
public:
	using LoadProcFn = void * (char const *);

	GraphicsApi(LoadProcFn * load_proc_fn);
	~GraphicsApi();

	void SetViewport(int width, int height) const;
};
