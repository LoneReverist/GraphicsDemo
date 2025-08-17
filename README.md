## Graphics Demo by LoneReverist
GraphicsDemo is a cross-platform C++ project showcasing modern graphics API design and software architecture, built using C++20, CMake, Docker, and supporting both OpenGL and Vulkan rendering back-ends.

![](https://github.com/chill-gamer-dev/GraphicsDemo/blob/master/OpenGLDemo.gif)
![](https://github.com/chill-gamer-dev/GraphicsDemo/blob/master/VulkanDemo.gif)

## Key Features

- Matching OpenGL and Vulkan implementations with interchangeable interfaces
- Graphical demos showcasing custom render pipelines, mesh loading, lighting and MSDF text rendering
- CMake + vcpkg build system
- Docker Linux builds
- Utilizes modern C++20 features, modules, and concepts for type safety and maintainability
- Modular Architecture, clear separation of platform, rendering and scene logic

## Project Structure
- DemoShared/
	- Core modules for asset management, mesh loading, font loading, and utility functions.
- OpenGLRenderer/ & VulkanRenderer/
	- Backend-specific implementations for graphics API abstraction, mesh, pipeline, and renderer logic.
- OpenGLDemo/
	- OpenGL-based demo application, shaders, and pipelines.
- VulkanDemo/
	- Vulkan-based demo application, shaders, and pipelines.

## Build for Windows
1. Install vcpkg and the libraries used by OpenGLDemo and VulkanDemo.
```
buildtools\Install-Dependencies.ps1
```

2. Install CMake, minimum version 3.29 for C++20 module support
3. Generate Visual Studio files and build OpenGLDemo and VulkanDemo Release executables
```
buildtools\Run-CMake.ps1
```

4. Run the demos
```
build\OpenGLDemo\Release\OpenGLDemo.exe
build\VulkanDemo\Release\VulkanDemo.exe
```

## Build for Linux
1. Install Docker
2. Run docker build
```
docker build -t graphicsdemo -f buildtools/Dockerfile .
```