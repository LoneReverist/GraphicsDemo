cmake_minimum_required(VERSION 3.28)

project(VulkanDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME})

# Build shaders
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

set(SHADER_SOURCES
	VulkanDemo/shaders/light_source.vert
	VulkanDemo/shaders/light_source.frag
	VulkanDemo/shaders/color.vert
	VulkanDemo/shaders/color.frag
	VulkanDemo/shaders/texture.vert
	VulkanDemo/shaders/texture.frag
	VulkanDemo/shaders/skybox.vert
	VulkanDemo/shaders/skybox.frag
	VulkanDemo/shaders/reflection.vert
	VulkanDemo/shaders/reflection.frag
)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(OUTPUT_FILE ${SHADER_BINARY_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${OUTPUT_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER} to SPIR-V"
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(${PROJECT_NAME} Shaders)

# Target Source Files
target_sources(${PROJECT_NAME}
	PRIVATE
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS
		VulkanDemo
	FILES
		VulkanDemo/Camera.ixx
		VulkanDemo/GraphicsApi.ixx
		VulkanDemo/GraphicsPipeline.ixx
		VulkanDemo/Input.ixx
		VulkanDemo/Mesh.ixx
		VulkanDemo/ObjLoader.ixx
		VulkanDemo/PipelineBuilder.ixx
		VulkanDemo/PlatformUtils.ixx
		VulkanDemo/Renderer.ixx
		VulkanDemo/RenderObject.ixx
		VulkanDemo/Scene.ixx
		VulkanDemo/Texture.ixx
		VulkanDemo/Vertex.ixx
		VulkanDemo/VulkanApp.ixx
)
target_sources(${PROJECT_NAME}
	PRIVATE
		VulkanDemo/GraphicsApi.cpp
		VulkanDemo/GraphicsPipeline.cpp
		VulkanDemo/ObjLoader.cpp
		VulkanDemo/PipelineBuilder.cpp
		VulkanDemo/Renderer.cpp
		VulkanDemo/Scene.cpp
		VulkanDemo/Texture.cpp
		VulkanDemo/VulkanApp.cpp
		VulkanDemo/VulkanDemo.cpp
)

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan glm::glm)

# Add stbi include directory, (this should perhaps be installed with vcpkg instead)
target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/stbi/include)

# Copy resources into output folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SHADER_BINARY_DIR}"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders"
)
