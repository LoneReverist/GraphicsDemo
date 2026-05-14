add_library(VulkanRenderer STATIC)

target_compile_features(VulkanRenderer PRIVATE cxx_std_23)
target_compile_definitions(VulkanRenderer PUBLIC
	VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1 # recommended by vulkan documentation
	#VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS=1 # Vulkan documetation recommends treating this exception as a success, but this is not implemented yet
	VULKAN_HPP_NO_SPACESHIP_OPERATOR=1 # Ninja has issues with the spaceship operator usage in vulkan
)
set_target_properties(VulkanRenderer PROPERTIES CXX_SCAN_FOR_MODULES ON)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(VulkanRenderer
	PUBLIC
	FILE_SET cxx_modules TYPE CXX_MODULES
	BASE_DIRS .
	FILES ${MODULE_FILES}
	PRIVATE ${SOURCE_FILES}
)

# Link dependencies (provided via vcpkg toolchain)
target_link_libraries(VulkanRenderer PUBLIC
	Vulkan::Vulkan
	glm::glm
)
