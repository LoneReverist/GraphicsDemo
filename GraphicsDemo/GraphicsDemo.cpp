// GraphicsDemo.cpp

import <iostream>;

import GLApp;

constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;

int main()
{
    std::cout << "Initializing glfw..." << std::endl;

    GLApp app(WINDOW_WIDTH, WINDOW_HEIGHT, "Graphics Demo");
    if (!app.IsInitialized() || !app.HasWindow())
        return -1;

    std::cout << "Running app..." << std::endl;

    app.Run();
}
