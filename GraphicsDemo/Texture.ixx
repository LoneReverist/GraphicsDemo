// Texture.ixx

module;

#include "stdafx.h"

export module Texture;

export class Texture
{
public:
	bool LoadTexture(std::filesystem::path const & filepath);
	bool LoadCubeMap(std::array<std::filesystem::path, 6> const & filepaths);

	void Bind() const;

private:
	int m_type{ -1 };
	unsigned int m_tex_id{ 0 };
};
