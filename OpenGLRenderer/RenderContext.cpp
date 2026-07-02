// RenderContext.cpp

module;

#include <cstddef>
#include <expected>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <glad/glad.h>

module Dreamhearth;

import :RenderContext;

namespace Dreamhearth
{
	namespace
	{
		GraphicsDiagnosticSeverity to_diagnostic_severity(GLenum type, GLenum severity)
		{
			if (type == GL_DEBUG_TYPE_ERROR || severity == GL_DEBUG_SEVERITY_HIGH)
				return GraphicsDiagnosticSeverity::Error;
			if (severity == GL_DEBUG_SEVERITY_MEDIUM || severity == GL_DEBUG_SEVERITY_LOW)
				return GraphicsDiagnosticSeverity::Warning;
			return GraphicsDiagnosticSeverity::Info;
		}

		void report_diagnostic(
			GraphicsDiagnosticFn const * callback,
			GraphicsDiagnostic diagnostic) noexcept
		{
			if (!callback || !*callback)
				return;

			try
			{
				(*callback)(diagnostic);
			}
			catch (...)
			{
				// Application callbacks must not throw through the OpenGL C API.
			}
		}
	}

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

		auto const * callback = static_cast<GraphicsDiagnosticFn const *>(userParam);
		report_diagnostic(callback, {
			.severity = to_diagnostic_severity(type, severity),
			.message = std::format("OpenGL {}: type - {}, id - {}\nMessage: {}",
				severity_to_string(severity), type_to_string(type), id,
				std::string_view{ message, static_cast<std::size_t>(length) })
		});
	}

	std::expected<RenderContext, GraphicsError> RenderContext::Create(
		int width_pixels,
		int height_pixels,
		LoadProcFn * load_proc_fn,
		GraphicsDiagnosticFn on_diagnostic)
	{
		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(load_proc_fn)))
			return std::unexpected{ GraphicsError{ "Failed to initialize OpenGL context" } };

		RenderContext render_context{ width_pixels, height_pixels, std::move(on_diagnostic) };
	
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(debug_message_callback, render_context.m_on_diagnostic.get());
	
		glEnable(GL_FRAMEBUFFER_SRGB);

		return render_context;
	}

	RenderContext::RenderContext(int width_pixels, int height_pixels, GraphicsDiagnosticFn on_diagnostic)
		: m_on_diagnostic(std::make_shared<GraphicsDiagnosticFn>(std::move(on_diagnostic)))
		, m_swap_chain_extent{
			width_pixels > 0 ? static_cast<std::uint32_t>(width_pixels) : 0,
			height_pixels > 0 ? static_cast<std::uint32_t>(height_pixels) : 0
		}
	{
	}

	void RenderContext::ReportDiagnostic(GraphicsDiagnostic diagnostic) const noexcept
	{
		report_diagnostic(m_on_diagnostic.get(), std::move(diagnostic));
	}
	
	RenderContext::~RenderContext()
	{
		if (m_on_diagnostic)
			glDebugMessageCallback(nullptr, nullptr);
	}
} // namespace Dreamhearth
