Push-Location $PSScriptRoot/..

New-Item -ItemType Directory -Force -Path "build"
cd build
cmake -B . -S .. --preset "x64-windows-static"
cmake --build . --config Release

Pop-Location

# Linux
# mkdir build-debug
# cd build-debug
# cmake -B . -S .. --preset "x64-linux" -DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19 -DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=/usr/lib/llvm-19/bin/clang-scan-deps -DCMAKE_CXX_COMPILER_CLANG_RESOURCE_DIR=/usr/lib/llvm-19/lib/clang/19 -DCMAKE_BUILD_TYPE=Debug
# cmake --build .
