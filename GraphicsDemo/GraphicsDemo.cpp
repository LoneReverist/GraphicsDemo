// GraphicsDemo.cpp

#include "stdafx.h"

#include "GLApp.h"

int main()
{
    std::cout << "Initializing glfw..." << std::endl;

    GLApp app(640 /*window_width*/, 480 /*window_height*/, "Graphics Demo");
    if (!app.IsInitialized() || !app.HasWindow())
        return -1;

    std::cout << "Running app..." << std::endl;

    app.Run();
}
