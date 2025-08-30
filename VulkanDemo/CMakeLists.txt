add_executable(VulkanDemo)

target_compile_features(VulkanDemo PRIVATE cxx_std_20)

set_target_properties(VulkanDemo PROPERTIES CXX_SCAN_FOR_MODULES ON)

# Build shaders
if (WIN32)
	set(GLSLANG_VALIDATOR_EXECUTABLE ${CMAKE_SOURCE_DIR}/buildtools/glslangValidator.exe)
elseif(UNIX)
	set(GLSLANG_VALIDATOR_EXECUTABLE glslangValidator)
endif()

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*")

set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(OUTPUT_FILE ${SHADER_BINARY_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} -V ${SHADER} -o ${OUTPUT_FILE} -DBUILD_VULKAN
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILE_NAME} to SPIR-V"
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
endforeach()

add_custom_target(VulkanShaders ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VulkanDemo>/resources/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_BINARY_DIR}" "$<TARGET_FILE_DIR:VulkanDemo>/resources/shaders"
    DEPENDS ${COMPILED_SHADERS}
    COMMENT "Copying shaders to resources directory"
)
add_dependencies(VulkanDemo VulkanShaders)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx"
    "${CMAKE_SOURCE_DIR}/DemoShared/*.ixx"
)
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/DemoShared/*.cpp"
)

# Target Source Files
target_sources(VulkanDemo
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
target_link_libraries(VulkanDemo PRIVATE
	VulkanRenderer
	glfw
	glm::glm
	nlohmann_json::nlohmann_json
)

# Copy resources into output folder
add_custom_command(TARGET VulkanDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:VulkanDemo>/resources"
)
