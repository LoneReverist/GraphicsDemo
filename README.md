## Graphics Demo by LoneReverist

![](https://github.com/chill-gamer-dev/GraphicsDemo/blob/master/GraphicsDemo.gif)

## Build:
1. Run Install-Dependencies.ps1, this will install vcpkg, glfw, glm and vulkan.
```
build/Install-Dependencies.ps1
```

2. Run Run-CMake.ps1, this has some default settings for lib locations and will generate Visual Studio files
```
build/Run-CMake.ps1
```

3. Build the Vulkan Demo project
```
cmake --build build --config Release
```

4. Run the Vulkan demo
```
build/Release/GraphicsDemo.exe
```
