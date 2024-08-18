// ObjLoader.h

#pragma once

struct BasicVertex;

namespace ObjLoader
{
	bool LoadObjFile(std::filesystem::path const & filepath, std::vector<BasicVertex> & out_vertices, std::vector<unsigned int> & out_indices);
}
