// AssetId.ixx

module;

export module AssetId;

export struct AssetId
{
	int m_index = -1;

	bool IsValid() { return m_index > 0; }
};
