find_package(glad CONFIG REQUIRED)

add_library(OpenGLRenderer STATIC)

target_compile_features(OpenGLRenderer PRIVATE cxx_std_20)

set_target_properties(OpenGLRenderer PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_sources(OpenGLRenderer
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
target_link_libraries(OpenGLRenderer PUBLIC
	glad::glad
	glfw
	glm::glm
)

# Add stbi include directory, (TODO: install with vcpkg)
target_include_directories(OpenGLRenderer PRIVATE
	${CMAKE_SOURCE_DIR}/stbi/include
)
