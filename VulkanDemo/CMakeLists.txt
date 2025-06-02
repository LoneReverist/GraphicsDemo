add_executable(VulkanDemo)

target_compile_features(VulkanDemo PRIVATE cxx_std_20)

set_target_properties(VulkanDemo PROPERTIES CXX_SCAN_FOR_MODULES ON)

# Build shaders
if (WIN32)
	set(GLSLANG_VALIDATOR_EXECUTABLE ${CMAKE_SOURCE_DIR}/buildtools/glslangValidator.exe)
elseif(UNIX)
	set(GLSLANG_VALIDATOR_EXECUTABLE glslangValidator)
endif()

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/VulkanDemo/shaders)
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

set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(OUTPUT_FILE ${SHADER_BINARY_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} -V ${SHADER} -o ${OUTPUT_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILE_NAME} to SPIR-V"
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(VulkanDemo Shaders)

# Target Source Files
target_sources(VulkanDemo
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
		VulkanApp.ixx

	PRIVATE
		ObjLoader.cpp
		Scene.cpp
		VulkanApp.cpp
		VulkanDemo.cpp

	PRIVATE
		${SHADER_SOURCES}
)
source_group("Shaders" FILES ${SHADER_SOURCES})

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(VulkanDemo PRIVATE
	glfw
	glm::glm
	VulkanRenderer
)

# Copy resources into output folder
add_custom_command(TARGET VulkanDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:VulkanDemo>/resources"
)

add_custom_command(TARGET VulkanDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SHADER_BINARY_DIR}"
    "$<TARGET_FILE_DIR:VulkanDemo>/resources/shaders"
)
