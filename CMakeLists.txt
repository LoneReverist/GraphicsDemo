cmake_minimum_required(VERSION 3.28) # for C++20 module support

project(GraphicsDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 3.4 REQUIRED)
find_package(glm 1.0.1 REQUIRED)

add_library(Vulkan::Vulkan STATIC IMPORTED)
set_target_properties(Vulkan::Vulkan PROPERTIES
    IMPORTED_LOCATION "${VCPKG_ROOT}/installed/x64-windows-static/lib/vulkan-1.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${VCPKG_ROOT}/installed/x64-windows-static/include"
)

add_subdirectory(VulkanRenderer)
add_subdirectory(VulkanDemo)
