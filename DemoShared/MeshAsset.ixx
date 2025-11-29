// MeshAsset.ixx

module;

#include <expected>
#include <filesystem>
#include <optional>
#include <vector>

export module MeshAsset;

import AssetId;
import GraphicsApi;
import GraphicsError;
import Mesh;
import ObjLoader;
import Vertex;

export template<IsVertex T>
class MeshAsset
{
public:
	using VertexT = T;

	static std::expected<Mesh, GraphicsError> Create(
		GraphicsApi const & graphics_api,
		std::vector<VertexT> const & vertices,
		std::vector<Mesh::IndexT> const & indices);

	static std::expected<Mesh, GraphicsError> Create(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & file_path);

	MeshAsset() = default;
	explicit MeshAsset(AssetId asset_id) : m_asset_id(asset_id) {}

	AssetId GetAssetId() const { return m_asset_id; }

private:
	AssetId m_asset_id;
};

template<IsVertex VertexT>
std::expected<Mesh, GraphicsError> MeshAsset<VertexT>::Create(
	GraphicsApi const & graphics_api,
	std::vector<VertexT> const & vertices,
	std::vector<Mesh::IndexT> const & indices)
{
	Mesh mesh{ graphics_api };
	std::expected<void, GraphicsError> result = mesh.Create(vertices, indices);
	if (!result.has_value())
		return std::unexpected{ result.error().AddToMessage(" MeshAsset::Create: Failed to create mesh.") };

	return std::move(mesh);
}

template<IsVertex VertexT>
std::expected<Mesh, GraphicsError> MeshAsset<VertexT>::Create(
	GraphicsApi const & graphics_api,
	std::filesystem::path const & file_path)
{
	if (!std::filesystem::exists(file_path))
		return std::unexpected{ GraphicsError{ " MeshAsset::Create: File does not exist: " + file_path.string() } };

	std::vector<VertexT> verts;
	std::vector<Mesh::IndexT> indices;
	if (!ObjLoader::LoadObjFile(file_path, verts, indices))
		return std::unexpected{ GraphicsError{ " MeshAsset::Create: Error loading file: " + file_path.string() } };

	return Create(graphics_api, verts, indices);
}
