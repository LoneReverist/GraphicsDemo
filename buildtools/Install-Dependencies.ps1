# clone vcpkg repo
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg

# run bootstrap script to build vcpkg.exe
Push-Location C:\vcpkg
.\bootstrap-vcpkg.bat

# integrate vcpkg with Visual Studio
.\vcpkg integrate install

# install dependencies
.\vcpkg install `
    glfw3:x64-windows-static `
    glm:x64-windows-static `
    vulkan:x64-windows-static `
    vulkan-validationlayers:x64-windows-static `
    glad:x64-windows-static `
    stb:x64-windows-static `
    nlohmann-json:x64-windows-static

# add vcpkg to path
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\vcpkg", [EnvironmentVariableTarget]::Machine)

#add VK_LAYER_PATH environment variable
[Environment]::SetEnvironmentVariable("VK_LAYER_PATH", "C:\vcpkg\installed\x64-windows-static\bin", [EnvironmentVariableTarget]::Machine)

Pop-Location
