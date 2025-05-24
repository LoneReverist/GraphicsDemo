find_package(glad CONFIG REQUIRED)

add_library(OpenGLRenderer STATIC)

target_sources(OpenGLRenderer
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
target_sources(OpenGLRenderer
	PRIVATE
		GraphicsApi.cpp
		GraphicsPipeline.cpp
		PipelineBuilder.cpp
		Renderer.cpp
		Texture.cpp
)

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(OpenGLRenderer PUBLIC
	glfw
	glm::glm
	glad::glad
)

# Add stbi include directory, (TODO: install with vcpkg)
target_include_directories(OpenGLRenderer PRIVATE
	${CMAKE_SOURCE_DIR}/stbi/include
)
