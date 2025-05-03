Push-Location $PSScriptRoot

cmake -B . -S .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" -Dglfw3_DIR="C:/vcpkg/installed/x64-windows-static/share/glfw3"
#cmake --build . --config Release

Pop-Location