add_library(GlfwVkWindow STATIC)

target_compile_features(GlfwVkWindow PUBLIC cxx_std_23)
set_target_properties(GlfwVkWindow PROPERTIES CXX_SCAN_FOR_MODULES ON)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(GlfwVkWindow
    PUBLIC
    FILE_SET CXX_MODULES TYPE CXX_MODULES
    BASE_DIRS .
    FILES ${MODULE_FILES}
    PRIVATE ${SOURCE_FILES}
)

target_link_libraries(GlfwVkWindow PUBLIC
    Vulkan::Vulkan
    glfw
    VulkanRenderer
)
