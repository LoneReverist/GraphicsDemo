// Texture.h

#pragma once

class Texture
{
public:
	bool LoadTexture(std::filesystem::path filepath);

	void Bind() const;

private:
	unsigned int m_tex_id{ 0 };
};
