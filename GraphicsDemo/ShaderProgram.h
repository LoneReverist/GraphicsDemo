// ShaderProgram.h

#pragma once
class ShaderProgram
{
public:
	~ShaderProgram();

	void LoadShaders(std::filesystem::path const & vert_shader_path, std::filesystem::path const & frag_shader_path);
	void Activate() const;

private:
	unsigned int load_shader(int type, std::filesystem::path const & shader_path) const;
	std::string read_file(std::filesystem::path const & path) const;

private:
	unsigned int m_program_id{ 0 };
};
