#add_library(Vulkan::Vulkan STATIC IMPORTED)
#set_target_properties(Vulkan::Vulkan PROPERTIES
#    IMPORTED_LOCATION "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/vulkan-1.lib"
#    INTERFACE_INCLUDE_DIRECTORIES "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include"
#)
find_package(Vulkan REQUIRED)

add_library(VulkanRenderer STATIC)

target_compile_features(VulkanRenderer PRIVATE cxx_std_20)

set_target_properties(VulkanRenderer PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_sources(VulkanRenderer
	PUBLIC
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS
		.
	FILES
		GraphicsApi.ixx
		GraphicsPipeline.ixx
		ImageData.ixx
		Mesh.ixx
		PipelineBuilder.ixx
		Renderer.ixx
		RenderObject.ixx
		Texture.ixx
		Vertex.ixx

	PRIVATE
		GraphicsApi.cpp
		GraphicsPipeline.cpp
		PipelineBuilder.cpp
		Renderer.cpp
		Texture.cpp
)

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(VulkanRenderer PUBLIC
	Vulkan::Vulkan
	glfw
	glm::glm
)

# Add stbi include directory, (this should perhaps be installed with vcpkg instead)
target_include_directories(VulkanRenderer PRIVATE
	${CMAKE_SOURCE_DIR}/stbi/include
)
