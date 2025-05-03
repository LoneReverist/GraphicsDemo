cmake_minimum_required(VERSION 3.28)

project(GraphicsDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
	PRIVATE
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS
		VulkanDemo
	FILES
		VulkanDemo/Camera.ixx
		VulkanDemo/GraphicsApi.ixx
		VulkanDemo/GraphicsPipeline.ixx
		VulkanDemo/Input.ixx
		VulkanDemo/Mesh.ixx
		VulkanDemo/ObjLoader.ixx
		VulkanDemo/PipelineBuilder.ixx
		VulkanDemo/PlatformUtils.ixx
		VulkanDemo/Renderer.ixx
		VulkanDemo/RenderObject.ixx
		VulkanDemo/Scene.ixx
		VulkanDemo/Texture.ixx
		VulkanDemo/Vertex.ixx
		VulkanDemo/VulkanApp.ixx
)
target_sources(${PROJECT_NAME}
	PRIVATE
		VulkanDemo/GraphicsApi.cpp
		VulkanDemo/GraphicsPipeline.cpp
		VulkanDemo/ObjLoader.cpp
		VulkanDemo/PipelineBuilder.cpp
		VulkanDemo/Renderer.cpp
		VulkanDemo/Scene.cpp
		VulkanDemo/Texture.cpp
		VulkanDemo/VulkanApp.cpp
		VulkanDemo/VulkanDemo.cpp
)

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan)

# Add GLM and stbi include directories (since they're header-only)
target_include_directories(${PROJECT_NAME} PRIVATE
    $ENV{VCPKG_ROOT}/installed/x64-windows/include
	${CMAKE_CURRENT_SOURCE_DIR}/stbi/include)

# copy resources into output folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/VulkanDemo/shaders/bin"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders"
)