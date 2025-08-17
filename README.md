## Graphics Demo by LoneReverist
GraphicsDemo is a cross-platform C++ project showcasing modern graphics API design and architecture, built using C++20, CMake, Docker, and supporting both OpenGL and Vulkan rendering back-ends.

![](https://github.com/chill-gamer-dev/GraphicsDemo/blob/master/OpenGLDemo.gif)
![](https://github.com/chill-gamer-dev/GraphicsDemo/blob/master/VulkanDemo.gif)

## Key Features

- Matching OpenGL and Vulkan implementations with interchangeable interfaces
- Graphical demos showcasing custom render pipelines, mesh loading, lighting and MSDF text rendering
- CMake + vcpkg build system
- Linux docker build
- Utilizes modern C++20 features, modules, and concepts for type safety and maintainability
- Modular architecture, clear separation of platform, rendering and scene logic

## Project Structure
- OpenGLRenderer/ & VulkanRenderer/
	- Static libraries implementing OpenGL and Vulkan rendering backends
	- Abstractions for mesh, texture, pipeline, and renderer logic
- OpenGLDemo/ & VulkanDemo/
	- Demo applications utilizing OpenGLRenderer and VulkanRenderer
	- Implements window creation, update/render loop, scene and custom render pipelines
- DemoShared/
	- Core modules for asset management, mesh loading, font loading, and utility functions
- buildtools/
	- Scripts for installing dependencies and running cmake, linux docker build
- resources/
	- Font, texture and object files

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