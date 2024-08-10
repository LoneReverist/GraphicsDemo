// ObjLoader.h

#pragma once

class Mesh;

namespace ObjLoader
{
	bool LoadObjFile(std::filesystem::path const & filepath, Mesh & mesh);
}
