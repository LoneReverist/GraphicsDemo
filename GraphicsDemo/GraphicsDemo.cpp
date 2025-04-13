// GraphicsDemo.cpp

#include <iostream>

import GLApp;

int main()
{
	std::cout << "Initializing app..." << std::endl;

	GLApp app(WindowSize{ 1920, 1080 }, "Graphics Demo");
	if (!app.IsInitialized() || !app.HasWindow())
		return -1;

	std::cout << "Running app..." << std::endl;

	app.Run();
}
