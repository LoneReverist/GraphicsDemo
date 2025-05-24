cmake_minimum_required(VERSION 3.28) # for C++20 module support

project(GraphicsDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
#find_package(stb CONFIG REQUIRED) # not needed, stb headers are globally available when vcpkg toolchain is set

add_subdirectory(VulkanRenderer)
add_subdirectory(VulkanDemo)

add_subdirectory(OpenGLRenderer)
add_subdirectory(OpenGLDemo)
