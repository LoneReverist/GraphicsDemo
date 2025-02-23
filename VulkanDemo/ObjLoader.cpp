// ObjLoader.cpp

module;

#include <array>
#include <fstream>
#include <ranges>
#include <unordered_map>

module ObjLoader;

//import <array>;
//import <fstream>; // not working?
//import <ranges>;
//import <unordered_map>;

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
	auto get_tokens(std::string_view str, std::string_view delim)
	{
		return std::views::split(str, delim)
			| std::views::filter([](auto token) { return std::ranges::distance(token) > 0; })
			| std::views::transform([](auto token) { return std::string_view(&*token.begin(), std::ranges::distance(token)); });
	}

	float to_float(std::string_view str)
	{
		float result;
		auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
		if (ec != std::errc{})
			throw std::runtime_error("Failed to parse float from obj file.");
		return result;
	}

	unsigned int to_uint(std::string_view str)
	{
		unsigned int result;
		auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
		if (ec != std::errc{})
			throw std::runtime_error("Failed to parse unsigned int from obj file.");
		return result;
	}

	bool read_obj_file(std::filesystem::path filepath,
		std::vector<std::array<float, 3>> & positions,
		std::vector<std::array<float, 3>> & normals,
		std::vector<ObjFaceVerts> & face_verts)
	{
		std::ifstream obj_file;
		obj_file.open(filepath);
		if (!obj_file.is_open())
			return false;

		std::string line;
		while (std::getline(obj_file, line))
		{
			auto tokens = get_tokens(line, " ");
			if (tokens.empty())
				continue;

			auto token_iter = tokens.begin();
			auto next_token = [&token_iter, &tokens]() -> std::string_view
				{
					if (token_iter == tokens.end())
						throw std::runtime_error("Unexpected number of tokens when reading obj file.");
					return *token_iter++;
				};

			std::string_view element_type = next_token();
			if (element_type == "v") // vertex position
			{
				std::array<float, 3> & pos = positions.emplace_back();
				pos[0] = to_float(next_token());
				pos[1] = to_float(next_token());
				pos[2] = to_float(next_token());
			}
			else if (element_type == "vn") // vertex normal
			{
				std::array<float, 3> & norm = normals.emplace_back();
				norm[0] = to_float(next_token());
				norm[1] = to_float(next_token());
				norm[2] = to_float(next_token());
			}
			else if (element_type == "f") // face
			{
				ObjFaceVerts & verts = face_verts.emplace_back();
				while (token_iter != tokens.end())
				{
					std::string_view vert_str = next_token();

					auto vert_tokens = get_tokens(vert_str, "/");
					if (std::ranges::distance(vert_tokens) != 2)
						throw std::runtime_error("Unexpected number of vertex tokens when reading obj file.");

					unsigned int pos_i = to_uint(*vert_tokens.begin());
					unsigned int norm_i = to_uint(*std::next(vert_tokens.begin()));
					verts.emplace_back(pos_i, norm_i);
				}
			}
		}

		obj_file.close();
		return true;
	}

	bool LoadObjFile(
		std::filesystem::path const & filepath,
		std::vector<NormalVertex> & out_vertices,
		std::vector<Mesh::index_t> & out_indices)
	{
		std::vector<std::array<float, 3>> positions;
		std::vector<std::array<float, 3>> normals;
		std::vector<ObjFaceVerts> face_verts;
		if (!read_obj_file(filepath, positions, normals, face_verts))
			return false;

		// Keep track of vertices we've already created so they can be reused.
		std::unordered_map<ObjVertex, unsigned int> vert_index_map;

		auto add_vert = [&](ObjVertex const & vert) -> unsigned int
			{
				auto iter = vert_index_map.find(vert);
				if (iter != vert_index_map.end())
					return iter->second;

				std::array<float, 3> const & pos = positions[vert.m_position_index - 1];
				std::array<float, 3> const & norm = normals[vert.m_normal_index - 1];
				out_vertices.push_back(NormalVertex{
					{ pos[0], pos[1], pos[2] },
					{ norm[0], norm[1], norm[2] }
					});

				return static_cast<Mesh::index_t>(out_vertices.size() - 1);
			};

		for (ObjFaceVerts const & verts : face_verts)
		{
			// verts are counter-clockwise, there may be more than 3 of them so we could be building multiple triangles
			std::vector<Mesh::index_t> vis;
			std::transform(verts.begin(), verts.end(), std::back_inserter(vis), add_vert);

			for (int i = 1; i + 2 <= vis.size(); i++)
			{
				out_indices.push_back(vis[0]);
				out_indices.push_back(vis[i]);
				out_indices.push_back(vis[i + 1]);
			}
		}

		return true;
	}
}
