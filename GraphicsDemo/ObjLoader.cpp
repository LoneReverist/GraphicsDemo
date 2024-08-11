// ObjLoader.cpp

#include "stdafx.h"
#include "ObjLoader.h"

#include "Mesh.h"

namespace
{
	struct ObjVertex
	{
		unsigned int m_position_index{ 0 };
		unsigned int m_normal_index{ 0 };

		bool operator==(ObjVertex const & other) const { return m_position_index == other.m_position_index && m_normal_index == other.m_normal_index; }
	};

	using ObjFaceVerts = std::vector<ObjVertex>;
}

template <>
struct std::hash<ObjVertex>
{
	size_t operator()(const ObjVertex & index) const
	{
		return std::hash<std::uint64_t>{}(index.m_position_index ^ (static_cast<std::uint64_t>(index.m_normal_index) << 32));
	}
};

namespace ObjLoader
{
	bool read_obj_file(std::filesystem::path filepath,
		std::vector<std::array<float, 3>> & positions,
		std::vector<std::array<float, 3>> & normals,
		std::vector<ObjFaceVerts> & face_verts)
	{
		std::ifstream obj_file;
		obj_file.open(filepath);
		if (!obj_file.is_open())
			return false;

		while (obj_file.good())
		{
			std::string token;
			obj_file >> token;

			if (token == "#") // comment
			{
				obj_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
			else if (token == "v") // vertex position
			{
				auto & pos = positions.emplace_back();
				obj_file >> pos[0] >> pos[1] >> pos[2];
			}
			else if (token == "vn") // vertex normal
			{
				auto & norm = normals.emplace_back();
				obj_file >> norm[0] >> norm[1] >> norm[2];
			}
			else if (token == "f") // face
			{
				std::string line;
				std::getline(obj_file, line);
				std::istringstream ss_line(line);

				ObjFaceVerts verts;
				for (std::string vert_token; std::getline(ss_line, vert_token, ' ');)
				{
					if (vert_token.empty())
						continue;

					char * context = nullptr;
					unsigned int pos_i = std::atoi(strtok_s(vert_token.data(), "/", &context));
					unsigned int norm_i = std::atoi(strtok_s(nullptr, "/", &context));
					verts.push_back(ObjVertex{ pos_i, norm_i });
				}
				face_verts.push_back(std::move(verts));
			}
		}

		obj_file.close();
		return true;
	}

	bool LoadObjFile(std::filesystem::path const & filepath, Mesh & mesh)
	{
		std::vector<std::array<float, 3>> positions;
		std::vector<std::array<float, 3>> normals;
		std::vector<ObjFaceVerts> face_verts;
		if (!read_obj_file(filepath, positions, normals, face_verts))
			return false;

		std::vector<Mesh::Vertex> out_vertices;
		std::vector<unsigned int> out_indices;

		// Keep track of vertices we've already created so they can be reused.
		std::unordered_map<ObjVertex, unsigned int> vert_index_map;

		auto add_vert = [&](ObjVertex const & vert) -> unsigned int
			{
				auto iter = vert_index_map.find(vert);
				if (iter != vert_index_map.end())
					return iter->second;

				std::array<float, 3> const & pos = positions[vert.m_position_index - 1];
				std::array<float, 3> const & norm = normals[vert.m_normal_index - 1];
				out_vertices.push_back(Mesh::Vertex{
					{ pos[0], pos[1], pos[2] },
					{ norm[0], norm[1], norm[2] }
					});

				return static_cast<unsigned int>(out_vertices.size() - 1);
			};

		for (ObjFaceVerts const & verts : face_verts)
		{
			// verts are counter-clockwise, there may be more than 3 of them so we could be building multiple triangles
			std::vector<unsigned int> vis;
			std::transform(verts.begin(), verts.end(), std::back_inserter(vis), add_vert);

			for (int i = 1; i + 2 <= vis.size(); i++)
			{
				out_indices.push_back(vis[0]);
				out_indices.push_back(vis[i]);
				out_indices.push_back(vis[i + 1]);
			}
		}

		mesh.SetVerts(std::move(out_vertices));
		mesh.SetIndices(std::move(out_indices));
		return true;
	}
}
