// AssimpLoader.ixx

module;

#include <expected>
#include <filesystem>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

export module AssimpLoader;

import GraphicsApi;
import GraphicsError;
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
			std::cout << "AssimpLoader::LoadObjWithColorMaterial: Failed to load file: " << filepath << std::endl;
			return {};
		}

		std::vector<Mesh> meshes;
		meshes.reserve(scene->mNumMeshes);
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh * ai_mesh = scene->mMeshes[i];
			const aiMaterial * material = scene->mMaterials[ai_mesh->mMaterialIndex];
			aiColor3D diffuse(1.0f, 1.0f, 1.0f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

			std::vector<ColorVertex> verts;
			std::vector<Mesh::IndexT> indices;
			verts.reserve(ai_mesh->mNumVertices);
			for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i)
			{
				aiVector3D pos = ai_mesh->mVertices[i];
				aiVector3D norm = ai_mesh->HasNormals() ? ai_mesh->mNormals[i] : aiVector3D(0, 0, 1);
				verts.push_back(ColorVertex{
					{ pos.x, pos.y, pos.z },
					{ norm.x, norm.y, norm.z },
					{ diffuse.r, diffuse.g, diffuse.b }
					});
			}
			for (unsigned int f = 0; f < ai_mesh->mNumFaces; ++f)
			{
				const aiFace & face = ai_mesh->mFaces[f];
				if (face.mNumIndices == 3) {
					indices.push_back(face.mIndices[0]);
					indices.push_back(face.mIndices[1]);
					indices.push_back(face.mIndices[2]);
				}
			}

			Mesh mesh{ graphics_api };
			std::expected<void, GraphicsError> result = mesh.Create(verts, indices);
			if (!result)
			{
				std::cout << "AssimpLoader::LoadObjWithColorMaterial: Failed to create mesh "
					<< i << " from: " << filepath << " Error: " << result.error().GetMessage() << std::endl;
				continue;
			}

			meshes.push_back(std::move(mesh));
		}
		return meshes;
	}
}
