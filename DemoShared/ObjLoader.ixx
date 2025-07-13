// ObjLoader.ixx

module;

#include <filesystem>
#include <vector>

export module ObjLoader;

import Mesh;
import Vertex;

export namespace ObjLoader
{
	bool LoadObjFile(
		std::filesystem::path const & filepath,
		std::vector<NormalVertex> & out_vertices,
		std::vector<Mesh::IndexT> & out_indices);
}
