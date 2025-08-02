cmake_minimum_required(VERSION 3.29) # for C++20 module support

option(BUILD_VULKAN "Build Vulkan renderer and demo projects" ON)
option(BUILD_OPENGL "Build OpenGL renderer and demo projects" ON)
option(BUILD_DEMOS "Build the demo executables in addition to the renderer libraries" ON)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON) # still required for gcc

project(GraphicsDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glm CONFIG REQUIRED)

if (BUILD_DEMOS)
    find_package(glfw3 CONFIG REQUIRED)
    #find_package(stb CONFIG REQUIRED) # not needed, stb headers are globally available when vcpkg toolchain is set
    find_package(nlohmann_json CONFIG REQUIRED)

    set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)
endif()

if(BUILD_VULKAN)
    add_subdirectory(VulkanRenderer)
    if (BUILD_DEMOS)
        add_subdirectory(VulkanDemo)
    endif()
endif()

if (BUILD_OPENGL)
    add_subdirectory(OpenGLRenderer)
    if (BUILD_DEMOS)
        add_subdirectory(OpenGLDemo)
    endif()
endif()
