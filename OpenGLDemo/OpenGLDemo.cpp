// OpenGLDemo.cpp

#include <iostream>

import OpenGLApp;

int main()
{
	std::cout << "Initializing app..." << std::endl;

	OpenGLApp app(WindowSize{ 1920, 1080 }, "OpenGL Demo");
	if (!app.IsInitialized() || !app.HasWindow())
		return -1;

	std::cout << "Running app..." << std::endl;

	app.Run();
}
