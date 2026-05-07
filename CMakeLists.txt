cmake_minimum_required(VERSION 3.29) # for C++20 module support

option(BUILD_VULKAN "Build Vulkan renderer and demo projects" ON)
option(BUILD_OPENGL "Build OpenGL renderer and demo projects" ON)
option(BUILD_DEMOS "Build the demo executables in addition to the renderer libraries" ON)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API ON) # still required for gcc
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for vscode c++ extension, allows it to find the compile commands for better intellisense and error checking

project(GraphicsDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else() # GCC/Clang
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

if(BUILD_VULKAN)
    find_package(Vulkan REQUIRED)

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	    # On Linux the vcpkg Vulkan loader doesn't have wayland support, so we need to link against the system loader instead
        find_library(VULKAN_SYSTEM_LOADER
           NAMES vulkan
           PATHS /usr/lib/x86_64-linux-gnu /usr/lib
            NO_DEFAULT_PATH
            REQUIRED)
        set_target_properties(Vulkan::Vulkan PROPERTIES
            IMPORTED_LOCATION ${VULKAN_SYSTEM_LOADER})
    endif()

    add_subdirectory(VulkanRenderer)
    add_subdirectory(GlfwVkWindow)
endif()

if(BUILD_OPENGL)
    find_package(glad CONFIG REQUIRED)

    add_subdirectory(OpenGLRenderer)
    add_subdirectory(GlfwGLWindow)
endif()

if(BUILD_DEMOS)
    #find_package(stb CONFIG REQUIRED) # not needed, stb headers are globally available when vcpkg toolchain is set
    find_package(nlohmann_json CONFIG REQUIRED)
    find_package(assimp CONFIG REQUIRED)

    set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

    add_subdirectory(Demo)
endif()
