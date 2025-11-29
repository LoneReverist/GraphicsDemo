find_package(glad CONFIG REQUIRED)

add_library(OpenGLRenderer STATIC)

target_compile_features(OpenGLRenderer PRIVATE cxx_std_23)

set_target_properties(OpenGLRenderer PROPERTIES CXX_SCAN_FOR_MODULES ON)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(OpenGLRenderer
	PUBLIC
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS
		.
	FILES
		${MODULE_FILES}

	PRIVATE
		${SOURCE_FILES}
)

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(OpenGLRenderer PUBLIC
	glad::glad
	glfw
	glm::glm
)
