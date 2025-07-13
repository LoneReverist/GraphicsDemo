add_executable(OpenGLDemo)

target_compile_features(OpenGLDemo PRIVATE cxx_std_20)

set_target_properties(OpenGLDemo PROPERTIES CXX_SCAN_FOR_MODULES ON)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*")

add_custom_target(OpenGLShaders ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OpenGLDemo>/resources/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_SOURCE_DIR}" "$<TARGET_FILE_DIR:OpenGLDemo>/resources/shaders"
    DEPENDS ${SHADER_FILES}
)
add_dependencies(OpenGLDemo OpenGLShaders)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx"
    "${CMAKE_SOURCE_DIR}/DemoShared/*.ixx"
)
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/DemoShared/*.cpp"
)

# Target Source Files
target_sources(OpenGLDemo
	PRIVATE
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS
		.
		"${CMAKE_SOURCE_DIR}/DemoShared"
	FILES
		${MODULE_FILES}

	PRIVATE
		${SOURCE_FILES}

	PRIVATE
		${SHADER_FILES}
)
source_group("Shaders" FILES ${SHADER_FILES})

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(OpenGLDemo PRIVATE
	OpenGLRenderer
	glfw
	glm::glm
	nlohmann_json::nlohmann_json
)

# Copy resources into output folder
add_custom_command(TARGET OpenGLDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:OpenGLDemo>/resources"
)
