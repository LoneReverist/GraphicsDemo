add_executable(OpenGLDemo)

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/OpenGLDemo/shaders)
set(SHADER_SOURCES
	${SHADER_SOURCE_DIR}/light_source.vert
	${SHADER_SOURCE_DIR}/light_source.frag
	${SHADER_SOURCE_DIR}/color.vert
	${SHADER_SOURCE_DIR}/color.frag
	${SHADER_SOURCE_DIR}/texture.vert
	${SHADER_SOURCE_DIR}/texture.frag
	${SHADER_SOURCE_DIR}/skybox.vert
	${SHADER_SOURCE_DIR}/skybox.frag
	${SHADER_SOURCE_DIR}/reflection.vert
	${SHADER_SOURCE_DIR}/reflection.frag
)

# Target Source Files
target_sources(OpenGLDemo
	PRIVATE
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS
		.
	FILES
		Camera.ixx
		Input.ixx
		ObjLoader.ixx
		PlatformUtils.ixx
		Scene.ixx
		OpenGLApp.ixx
)
target_sources(OpenGLDemo
	PRIVATE
		ObjLoader.cpp
		Scene.cpp
		OpenGLApp.cpp
		OpenGLDemo.cpp
)
target_sources(OpenGLDemo
	PRIVATE
	${SHADER_SOURCES}
)
source_group("Shaders" FILES ${SHADER_SOURCES})

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(OpenGLDemo PRIVATE
	glfw
	glm::glm
	OpenGLRenderer
)

# Copy resources into output folder
add_custom_command(TARGET OpenGLDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:OpenGLDemo>/resources"
)

add_custom_command(TARGET OpenGLDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SHADER_SOURCE_DIR}"
    "$<TARGET_FILE_DIR:OpenGLDemo>/resources/shaders"
)
