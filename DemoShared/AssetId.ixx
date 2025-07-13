// AssetId.ixx

module;

#include <concepts>

export module AssetId;

import Vertex;

export template <IsVertex T>
struct AssetId
{
	using VertexT = T;
	int m_index = -1;
};

export template <typename AssetId1, typename AssetId2>
concept AssetsAreCompatible = std::same_as<typename AssetId1::VertexT, typename AssetId2::VertexT>;
