// main.cpp

#include <iostream>

import App;

#ifdef BUILD_VULKAN
constexpr char const * AppName = "Vulkan Demo - Dreamhearth Engine";
#elif defined(BUILD_OPENGL)
constexpr char const * AppName = "OpenGL Demo - Dreamhearth Engine";
#else
#error "Either BUILD_VULKAN or BUILD_OPENGL must be defined"
#endif

namespace dh = Dreamhearth;

int main()
{
	std::cout << "Initializing app..." << std::endl;

	dh::App app(dh::WindowSize{ 1920, 1080 }, AppName);
	if (!app.IsInitialized() || !app.HasWindow())
		return -1;

	std::cout << "Running app..." << std::endl;

	app.Run();
}
