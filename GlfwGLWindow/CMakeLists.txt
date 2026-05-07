add_library(GlfwGLWindow STATIC)

target_compile_features(GlfwGLWindow PUBLIC cxx_std_23)
set_target_properties(GlfwGLWindow PROPERTIES CXX_SCAN_FOR_MODULES ON)

file(GLOB MODULE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(GlfwGLWindow
    PUBLIC
    FILE_SET CXX_MODULES TYPE CXX_MODULES
    BASE_DIRS .
    FILES ${MODULE_FILES}
    PRIVATE ${SOURCE_FILES}
)

target_link_libraries(GlfwGLWindow PUBLIC
    glfw
    OpenGLRenderer
)
