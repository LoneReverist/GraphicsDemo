add_library(Vulkan::Vulkan STATIC IMPORTED)
set_target_properties(Vulkan::Vulkan PROPERTIES
    IMPORTED_LOCATION "${VCPKG_ROOT}/installed/x64-windows-static/lib/vulkan-1.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${VCPKG_ROOT}/installed/x64-windows-static/include"
)

add_library(VulkanRenderer STATIC)

target_sources(VulkanRenderer
	PRIVATE
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS
		.
	FILES
		GraphicsApi.ixx
		GraphicsPipeline.ixx
		Mesh.ixx
		PipelineBuilder.ixx
		Renderer.ixx
		RenderObject.ixx
		Texture.ixx
		Vertex.ixx
)
target_sources(VulkanRenderer
	PRIVATE
		GraphicsApi.cpp
		GraphicsPipeline.cpp
		PipelineBuilder.cpp
		Renderer.cpp
		Texture.cpp
)

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(VulkanRenderer PUBLIC
	glfw
	glm::glm
	Vulkan::Vulkan
)

# Add stbi include directory, (this should perhaps be installed with vcpkg instead)
target_include_directories(VulkanRenderer PRIVATE
	${CMAKE_SOURCE_DIR}/stbi/include
)
