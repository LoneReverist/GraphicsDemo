add_executable(VulkanDemo)

# Build shaders
set(GLSLANG_VALIDATOR_EXECUTABLE ${CMAKE_SOURCE_DIR}/buildtools/glslangValidator.exe)

set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

set(SHADER_SOURCES
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/light_source.vert
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/light_source.frag
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/color.vert
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/color.frag
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/texture.vert
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/texture.frag
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/skybox.vert
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/skybox.frag
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/reflection.vert
	${CMAKE_SOURCE_DIR}/VulkanDemo/shaders/reflection.frag
)

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
)
target_sources(VulkanDemo
	PRIVATE
		ObjLoader.cpp
		Scene.cpp
		VulkanApp.cpp
		VulkanDemo.cpp
)
target_sources(VulkanDemo
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
