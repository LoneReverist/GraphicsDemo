#add_library(Vulkan::Vulkan STATIC IMPORTED)
#set_target_properties(Vulkan::Vulkan PROPERTIES
#    IMPORTED_LOCATION "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/vulkan-1.lib"
#    INTERFACE_INCLUDE_DIRECTORIES "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include"
#)
find_package(Vulkan REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# On Linux the vcpkg Vulkan loader doesn't have wayland support, so we need to link against the system loader instead
    find_library(VULKAN_SYSTEM_LOADER
        NAMES vulkan
        PATHS /usr/lib/x86_64-linux-gnu /usr/lib
        NO_DEFAULT_PATH
        REQUIRED)
    set_target_properties(Vulkan::Vulkan PROPERTIES
        IMPORTED_LOCATION ${VULKAN_SYSTEM_LOADER})
endif()

add_library(VulkanRenderer STATIC)

target_compile_features(VulkanRenderer PRIVATE cxx_std_23)

target_compile_definitions(VulkanRenderer PUBLIC
	VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
	#VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS=1
)

set_target_properties(VulkanRenderer PROPERTIES CXX_SCAN_FOR_MODULES ON)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(VulkanRenderer
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
target_link_libraries(VulkanRenderer PUBLIC
	Vulkan::Vulkan
	glfw
	glm::glm
)
