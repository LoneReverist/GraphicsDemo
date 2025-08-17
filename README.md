## Graphics Demo by LoneReverist

![](https://github.com/chill-gamer-dev/GraphicsDemo/blob/master/GraphicsDemo.gif)

## Build (Windows):
1. Install vcpkg and the libraries used by OpenGLDemo and VulkanDemo.
```
buildtools/Install-Dependencies.ps1
```

2. Install CMake, minimum version 3.29 for C++20 module support
3. Generate Visual Studio files and build OpenGLDemo and VulkanDemo Release executables
```
buildtools/Run-CMake.ps1
```

4. Run the demos
```
build/OpenGLDemo/Release/OpenGLDemo.exe
build/VulkanDemo/Release/VulkanDemo.exe
```
