# clone vcpkg repo
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg

# run bootstrap script to build vcpkg.exe
Push-Location C:\vcpkg
.\bootstrap-vcpkg.bat

# integrate vcpkg with Visual Studio
.\vcpkg integrate install

# install VulkanDemo dependencies
.\vcpkg install glfw3:x64-windows-static glm:x64-windows vulkan:x64-windows-static glslang

# add vcpkg to path
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\vcpkg", [EnvironmentVariableTarget]::Machine)

Pop-Location
