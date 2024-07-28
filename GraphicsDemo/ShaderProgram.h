// ShaderProgram.h

#include <glm/gtc/matrix_transform.hpp>

#pragma once
class ShaderProgram
{
public:
	~ShaderProgram();

	void LoadShaders(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path);
	void Activate() const;

	void SetWorldTransform(glm::mat4 const & transform) const;

private:
	unsigned int load_shader(int type, std::filesystem::path const & shader_path) const;
	std::string read_file(std::filesystem::path const & path) const;

private:
	unsigned int m_program_id{ 0 };
};
