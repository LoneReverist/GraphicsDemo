// MeshAsset.ixx

module;

#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

export module MeshAsset;

import AssetId;
import GraphicsApi;
import Mesh;
import ObjLoader;
import Vertex;

export template<IsVertex T>
class MeshAsset
{
public:
	using VertexT = T;

	static std::optional<Mesh> Create(
		GraphicsApi const & graphics_api,
		std::vector<VertexT> const & vertices,
		std::vector<Mesh::IndexT> const & indices);

	static std::optional<Mesh> Create(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & file_path);

	MeshAsset() = default;
	explicit MeshAsset(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
};

template<IsVertex VertexT>
std::optional<Mesh> MeshAsset<VertexT>::Create(
	GraphicsApi const & graphics_api,
	std::vector<VertexT> const & vertices,
	std::vector<Mesh::IndexT> const & indices)
{
	return Mesh{ graphics_api, vertices, indices };
}

template<IsVertex VertexT>
std::optional<Mesh> MeshAsset<VertexT>::Create(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & file_path)
{
	if (!std::filesystem::exists(file_path))
	{
		std::cout << "NormalMesh::Create() file does not exist:" << file_path << std::endl;
		return std::nullopt;
	}

	std::vector<VertexT> verts;
	std::vector<Mesh::IndexT> indices;
	if (!ObjLoader::LoadObjFile(file_path, verts, indices))
	{
		std::cout << "create_mesh_from_file() error loading file:" << file_path << std::endl;
		return std::nullopt;
	}

	return Mesh{ graphics_api, verts, indices };
}
