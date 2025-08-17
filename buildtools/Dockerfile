FROM ubuntu:24.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        gnupg \
        curl \
        clang-18 \
        clang-tools-18 \
        lld-18 \
        ninja-build \
        git \
        pkg-config \
        zip \
        unzip \
        python3 \
        tar \
        libxinerama-dev \
        libxcursor-dev \
        xorg-dev \
        libglu1-mesa-dev \
        libvulkan-dev \
        vulkan-tools \
        vulkan-utility-libraries-dev \
        glslang-tools \
    && rm -rf /var/lib/apt/lists/*

# Add Kitware APT repo key and source list for the latest version of cmake
# This is necessary because ubuntu:24.04 comes with cmake 3.28, but we need at least 3.29 for reliable module support
RUN curl -fsSL https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        cmake \
    && rm -rf /var/lib/apt/lists/*

# Set clang-18 as default compiler
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-18 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-18 100

RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh

RUN /opt/vcpkg/vcpkg install \
    glfw3:x64-linux \
    glm:x64-linux \
    vulkan:x64-linux \
#    vulkan-validationlayers:x64-linux \
    glad:x64-linux \
    stb:x64-linux \
    nlohmann-json:x64-linux

WORKDIR /graphicsdemo
COPY . .

RUN mkdir -p build
WORKDIR /graphicsdemo/build

# building with clang-18 as it has better c++20 module support than gcc-14
RUN cmake -B . -S .. --preset "x64-linux" \
    -DCMAKE_C_COMPILER=clang-18 \
    -DCMAKE_CXX_COMPILER=clang++-18 \
    -DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=/usr/lib/llvm-18/bin/clang-scan-deps \
    -DCMAKE_CXX_COMPILER_CLANG_RESOURCE_DIR=/usr/lib/llvm-18/lib/clang/18
RUN cmake --build . --config Release
