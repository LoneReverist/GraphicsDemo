// GraphicsApi.cpp

module;

#include <format>
#include <iostream>

#include <glad/glad.h>

module GraphicsApi;

namespace
{
	std::string type_to_string(GLenum type)
	{
		switch (type) {
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		default: return "UNKNOWN";
		}
	}

	std::string severity_to_string(GLenum severity)
	{
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		default: return "UNKNOWN";
		}
	}

	void GLAPIENTRY debug_message_callback(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar * message,
		const void * userParam)
	{
		if (id == 131185) // ignore notification about using GL_STATIC_DRAW
			return;

		std::cout << std::format("OpenGL {3}: type - {1}, id - {2}\nMessage: {0}\n\n",
			message, type_to_string(type), id, severity_to_string(severity));
	}
}

GraphicsApi::GraphicsApi(LoadProcFn * load_proc_fn)
{
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(load_proc_fn)))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debug_message_callback, 0);
}

GraphicsApi::~GraphicsApi()
{
}
