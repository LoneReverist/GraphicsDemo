// ObjLoader.ixx

export module ObjLoader;

import <filesystem>;

import Mesh;

export namespace ObjLoader
{
	bool LoadObjFile(
		std::filesystem::path const & filepath,
		std::vector<NormalVertex> & out_vertices,
		std::vector<Mesh::index_t> & out_indices);
}
