## Graphics Demo by LoneReverist

![](https://github.com/chill-gamer-dev/GraphicsDemo/blob/master/GraphicsDemo.gif)

## Build:
1. Run buildtools/Install-Dependencies.ps1, this will install vcpkg and then packages used by OpenGLDemo and VulkanDemo.
```
buildtools/Install-Dependencies.ps1
```

2. Run buildtools/Run-CMake.ps1, this will generate Visual Studio files and build OpenGLDemo and VulkanDemo
```
buildtools/Run-CMake.ps1
```

3. Run the OpenGL/Vulkan demos
```
build/OpenGLDemo/Release/OpenGLDemo.exe
build/VulkanDemo/Release/VulkanDemo.exe
```
