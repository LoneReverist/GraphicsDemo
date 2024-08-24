// ShaderProgram.ixx

module;

#include "stdafx.h"

#include <glm/gtc/matrix_transform.hpp>

export module ShaderProgram;

export class ShaderProgram
{
public:
	bool LoadShaders(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path);
	void DeleteShaders();

	void Activate() const;
	void SetUniform(std::string const & uniform_label, float uniform) const;
	void SetUniform(std::string const & uniform_label, glm::vec3 const & uniform) const;
	void SetUniform(std::string const & uniform_label, glm::mat4 const & uniform) const;

private:
	unsigned int load_shader(int type, std::filesystem::path const & shader_path) const;
	std::string read_file(std::filesystem::path const & path) const;

private:
	unsigned int m_program_id{ 0 };
};
