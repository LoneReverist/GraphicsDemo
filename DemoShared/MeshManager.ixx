// MeshManager.ixx

module;

#include <expected>
#include <filesystem>
#include <vector>

export module MeshManager;

import AssetPool;
import GraphicsApi;
import GraphicsError;
import Mesh;
import ObjLoader;
import Vertex;

export template<IsVertex T>
class MeshId : public AssetId
{
public:
	using VertexT = T;
};

export class MeshManager
{
public:
	MeshManager(GraphicsApi const & graphics_api)
		: m_graphics_api{ graphics_api }
	{}

	template<IsVertex VertexT>
	std::expected<MeshId<VertexT>, GraphicsError> AddMesh(Mesh mesh);

	template<IsVertex VertexT>
	std::expected<MeshId<VertexT>, GraphicsError> CreateMesh(
		std::vector<VertexT> const & vertices,
		std::vector<Mesh::IndexT> const & indices);

	template<IsVertex VertexT>
	std::expected<MeshId<VertexT>, GraphicsError> CreateMesh(
		std::filesystem::path const & file_path);

	void Remove(AssetId id) { m_mesh_pool.Remove(id); }

	Mesh const * Get(AssetId id) const { return m_mesh_pool.Get(id); }
	Mesh * Get(AssetId id) { return m_mesh_pool.Get(id); }

private:
	GraphicsApi const & m_graphics_api;
	AssetPool<Mesh> m_mesh_pool;
};

template<IsVertex VertexT>
std::expected<MeshId<VertexT>, GraphicsError> MeshManager::AddMesh(Mesh mesh)
{
	MeshId<VertexT> mesh_id{ m_mesh_pool.Add(std::move(mesh)) };
	if (!mesh_id.IsValid())
		return std::unexpected{ GraphicsError{ "MeshManager::AddMesh: Failed to add mesh to pool." } };

	return mesh_id;
}

template<IsVertex VertexT>
std::expected<MeshId<VertexT>, GraphicsError> MeshManager::CreateMesh(
	std::vector<VertexT> const & vertices,
	std::vector<Mesh::IndexT> const & indices)
{
	Mesh mesh{ m_graphics_api };
	std::expected<void, GraphicsError> result = mesh.Create(vertices, indices);
	if (!result.has_value())
		return std::unexpected{ result.error().AddToMessage(" MeshManager::CreateMesh: Failed to create mesh.") };

	MeshId<VertexT> mesh_id{ m_mesh_pool.Add(std::move(mesh)) };
	if (!mesh_id.IsValid())
		return std::unexpected{ GraphicsError{ "MeshManager::CreateMesh: Failed to add mesh to pool." } };

	return mesh_id;
}

template<IsVertex VertexT>
std::expected<MeshId<VertexT>, GraphicsError> MeshManager::CreateMesh(
	std::filesystem::path const & file_path)
{
	if (!std::filesystem::exists(file_path))
		return std::unexpected{ GraphicsError{ "MeshManager::CreateMesh: File does not exist: " + file_path.string() } };

	std::vector<VertexT> verts;
	std::vector<Mesh::IndexT> indices;
	if (!ObjLoader::LoadObjFile(file_path, verts, indices))
		return std::unexpected{ GraphicsError{ "MeshManager::CreateMesh: Error loading file: " + file_path.string() } };

	return CreateMesh(verts, indices);
}
