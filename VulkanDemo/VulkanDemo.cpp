// VulkanDemo.cpp

#include <iostream>

//import <iostream>;

import VulkanApp;

int main()
{
	std::cout << "Initializing app..." << std::endl;

	VulkanApp app(WindowSize{ 1920, 1080 }, "Vulkan Demo");
	if (!app.IsInitialized() || !app.HasWindow())
		return -1;

	std::cout << "Running app..." << std::endl;

	app.Run();
}
