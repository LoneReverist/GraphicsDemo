// ObjLoader.ixx

module;

#include "stdafx.h"

export module ObjLoader;

import Mesh;

export namespace ObjLoader
{
	bool LoadObjFile(std::filesystem::path const & filepath, std::vector<NormalVertex> & out_vertices, std::vector<unsigned int> & out_indices);
}
