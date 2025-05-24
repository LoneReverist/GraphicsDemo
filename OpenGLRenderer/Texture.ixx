// Texture.ixx

module;

#include <array>
#include <filesystem>

export module Texture;

export class Texture
{
public:
	Texture(std::filesystem::path const & filepath);
	Texture(std::array<std::filesystem::path, 6> const & filepaths); // cubemap
	~Texture();

	Texture(Texture && other);
	Texture & operator=(Texture && other);

	Texture(Texture &) = delete;
	Texture & operator=(Texture &) = delete;

	bool IsValid() const;

	void Bind() const;

private:
	void destroy_texture();

private:
	unsigned int m_type{ 0 };
	unsigned int m_tex_id{ 0 };
};
