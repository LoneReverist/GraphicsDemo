// ObjLoader.ixx

module;

#include <filesystem>

export module ObjLoader;

import Mesh;
import Vertex;

export namespace ObjLoader
{
	bool LoadObjFile(
		std::filesystem::path const & filepath,
		std::vector<NormalVertex> & out_vertices,
		std::vector<Mesh::index_t> & out_indices);
}
