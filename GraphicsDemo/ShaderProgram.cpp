// ShaderProgram.cpp

#include "stdafx.h"
#include "ShaderProgram.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

bool ShaderProgram::LoadShaders(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path)
{
	unsigned int vert_shader_id = load_shader(GL_VERTEX_SHADER, vert_shader_path);
	unsigned int frag_shader_id = load_shader(GL_FRAGMENT_SHADER, frag_shader_path);
	if (vert_shader_id == 0 || frag_shader_id == 0)
	{
		glDeleteShader(vert_shader_id); // TODO: a scope_guard class would be handy
		glDeleteShader(frag_shader_id);
		return false;
	}

	m_program_id = glCreateProgram();
	glAttachShader(m_program_id, vert_shader_id);
	glAttachShader(m_program_id, frag_shader_id);
	glLinkProgram(m_program_id);

	int success = 0;
	glGetProgramiv(m_program_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char info_log[512];
		glGetProgramInfoLog(m_program_id, 512, nullptr, info_log);
		std::cout << "Failed to link shader program:\n" << info_log << std::endl;
	}

	glDeleteShader(vert_shader_id);
	glDeleteShader(frag_shader_id);
	return success;
}

void ShaderProgram::DeleteShaders()
{
	glDeleteProgram(m_program_id);
}

void ShaderProgram::Activate() const
{
	if (m_program_id == 0)
	{
		std::cout << "Activating invalid shader program" << std::endl;
		return;
	}

	glUseProgram(m_program_id);
}

void ShaderProgram::SetMat4(std::string const & uniform_label, glm::mat4 const & uniform) const
{
	unsigned int uniform_loc = glGetUniformLocation(m_program_id, uniform_label.c_str());
	glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(uniform));
}

unsigned int ShaderProgram::load_shader(int type, std::filesystem::path const & shader_path) const
{
	std::string file_data = read_file(shader_path);
	if (file_data.empty())
		return 0;

	const char * shader_source = file_data.c_str();

	unsigned int shader_id = glCreateShader(type); // returns 0 on error
	glShaderSource(shader_id, 1, &shader_source, nullptr);
	glCompileShader(shader_id);

	int success = 0;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char info_log[512];
		glGetShaderInfoLog(shader_id, 512, nullptr, info_log);
		std::cout << "Failed to compile shader. Type: " << std::hex << type << std::endl << info_log << std::endl;
	}

	return shader_id;
}

std::string ShaderProgram::read_file(std::filesystem::path const & path) const
{
	std::ifstream file(path.string().c_str());
	if (!file.is_open())
	{
		std::cout << "Failed to open file for reading: " << path;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	file.close();

	return buffer.str();
}
