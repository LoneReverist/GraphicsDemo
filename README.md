## Dreamhearth Engine by LoneReverist
Dreamhearth Engine is a cross-platform rendering engine showcasing modern graphics API design and architecture, built with C++20, CMake, Docker, and supporting both OpenGL and Vulkan back-ends.

![](OpenGLDemo.gif)
![](VulkanDemo.gif)

## Key Features

- Matching OpenGL and Vulkan back-ends can be used interchangeably
- Graphical demos showcasing custom render pipelines, mesh loading, lighting and MSDF text rendering
- Runs on both Windows and Linux
- CMake + vcpkg build system
- Utilizes modern C++20 features, modules, and concepts for type safety and maintainability
- Modular architecture, clear separation of platform, rendering and scene logic

## Project Structure
- OpenGLRenderer/ & VulkanRenderer/
	- Static libraries implementing OpenGL and Vulkan rendering backends
	- Abstractions for mesh, texture, pipeline, and renderer logic
- GlfwGLWindow/ & GlfwVkWindow/
	- Static libraries implementing window creation and render context creation with GLFW
- Demo/
	- Demo application, can be built with either OpenGL or Vulkan
	- Implements update/render loop and modules for the scene, input, asset loading, custom render pipelines, etc.
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
