// Buffer.cpp

module;

#include <iostream>
#include <cstdint>

#include <glad/glad.h>

module Buffer;

Buffer::~Buffer()
{
	Destroy();
}

Buffer::Buffer(Buffer && other) noexcept
	: m_id(other.m_id)
{
	other.m_id = 0;
}

Buffer & Buffer::operator=(Buffer && other) noexcept
{
	if (this != &other)
	{
		Destroy();

		std::swap(m_id, other.m_id);
	}
	return *this;
}

void Buffer::Create()
{
	Destroy();

	glGenBuffers(1, &m_id);
}

void Buffer::Destroy()
{
	if (m_id != 0)
		glDeleteBuffers(1, &m_id);
}
