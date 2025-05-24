Push-Location $PSScriptRoot/..

New-Item -ItemType Directory -Force -Path "build"
cd build
cmake -B . -S .. --preset "x64-windows-static"
cmake --build . --config Release

Pop-Location
