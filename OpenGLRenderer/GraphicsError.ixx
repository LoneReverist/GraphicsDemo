// GraphicsError.ixx

module;

#include <string>
#include <source_location>
#include <stacktrace>

export module GraphicsError;

export class GraphicsError
{
public:
	explicit GraphicsError(
		std::string message/*,
		std::source_location const loc = std::source_location::current(),
		std::stacktrace const trace = std::stacktrace::current()*/);

	std::string const & GetMessage() const { return m_message; }
	//std::source_location const & GetSourceLocation() const { return m_source_location; }
	//std::stacktrace const & GetStackTrace() const { return m_stack_trace; }

	GraphicsError & AddToMessage(std::string message) { m_message += message; return *this; }

private:
	std::string m_message;
	//std::source_location const m_source_location;
	//std::stacktrace const m_stack_trace;
};

static_assert(sizeof(GraphicsError) <= 64, "a type that is 64 bytes or less is optimal for std::expected usage");

GraphicsError::GraphicsError(
	std::string message)//,
	//std::source_location const loc /*= std::source_location::current()*/,
	//std::stacktrace const trace /*= std::stacktrace::current()*/)
	: m_message(std::move(message))
	//, m_source_location(loc)
	//, m_stack_trace(trace)
{
}
