// AssimpLoader.ixx

module;

#include <filesystem>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

export module AssimpLoader;

import GraphicsApi;
import Mesh;
import Vertex;

export namespace AssimpLoader
{
	std::vector<Mesh> LoadObjWithColorMaterial(
		GraphicsApi const & graphics_api,
		std::filesystem::path const & filepath)
	{
		Assimp::Importer importer;
		const aiScene * scene = importer.ReadFile(filepath.string(),
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
		if (!scene || !scene->HasMeshes())
		{
			std::cout << "Assimp failed to load: " << filepath << std::endl;
			return {};
		}

		std::vector<Mesh> meshes;
		meshes.reserve(scene->mNumMeshes);
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh * mesh = scene->mMeshes[i];
			const aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];
			aiColor3D diffuse(1.0f, 1.0f, 1.0f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

			std::vector<ColorVertex> verts;
			std::vector<Mesh::IndexT> indices;
			verts.reserve(mesh->mNumVertices);
			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				aiVector3D pos = mesh->mVertices[i];
				aiVector3D norm = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0, 0, 1);
				verts.push_back(ColorVertex{
					{ pos.x, pos.y, pos.z },
					{ norm.x, norm.y, norm.z },
					{ diffuse.r, diffuse.g, diffuse.b }
					});
			}
			for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
			{
				const aiFace & face = mesh->mFaces[f];
				if (face.mNumIndices == 3) {
					indices.push_back(face.mIndices[0]);
					indices.push_back(face.mIndices[1]);
					indices.push_back(face.mIndices[2]);
				}
			}

			meshes.push_back(Mesh{ graphics_api, verts, indices });
		}
		return meshes;
	}
}
