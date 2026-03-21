// AssetPool.ixx

module;

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

export module AssetPool;

constexpr std::uint32_t IndexMask = 0xFFFFF; // 20 bits for index
constexpr std::uint32_t GenerationMask = 0xFFF; // 12 bits for generation
constexpr std::uint32_t GenerationShift = 20; // generation starts at bit 20
constexpr std::uint32_t InvalidId = 0xFFFFFFFF; // reserved invalid ID

export class AssetId
{
public:
	AssetId() = default;
	AssetId(std::uint32_t index, std::uint32_t generation)
		: id((generation << GenerationShift) | index)
	{}

	std::uint32_t GetIndex() const { return id & IndexMask; }
	std::uint32_t GetGeneration() const { return id >> GenerationShift; }
	bool IsValid() { return id != InvalidId; }

	bool operator==(AssetId const & other) const { return id == other.id; }

private:
	std::uint32_t id = InvalidId;
};

struct AssetMeta
{
	std::uint32_t generation : 12 = 0;
	std::uint32_t is_active : 1 = 0;
	std::uint32_t not_used : 19 = 0; // padding to fill 32 bits
};

export template <typename AssetType>
	requires std::is_move_constructible_v<AssetType> && std::is_move_assignable_v<AssetType>
class AssetPool
{
public:
	AssetPool() = default;
	~AssetPool() = default;

	AssetPool(AssetPool && other) = default;
	AssetPool & operator=(AssetPool && other) = default;

	AssetPool(AssetPool const &) = delete;
	AssetPool & operator=(AssetPool const &) = delete;

	AssetId Add(AssetType asset);
	void Remove(AssetId id);

	AssetType const * Get(AssetId id) const;
	AssetType * Get(AssetId id);

private:
	std::vector<AssetMeta> m_meta;
	std::vector<AssetType> m_assets;

	std::vector<std::uint32_t> m_free_indices;
};

template <typename AssetType>
	requires std::is_move_constructible_v<AssetType> && std::is_move_assignable_v<AssetType>
AssetId AssetPool<AssetType>::Add(AssetType asset)
{
	std::uint32_t index;
	if (!m_free_indices.empty())
	{
		index = m_free_indices.back();
		m_free_indices.pop_back();

		m_assets[index] = std::move(asset);
	}
	else
	{
		index = static_cast<std::uint32_t>(m_assets.size());
		if (index >= IndexMask)
			return AssetId{};

		m_assets.push_back(std::move(asset));
		m_meta.emplace_back();
	}

	m_meta[index].is_active = 1;

	return AssetId{ index, m_meta[index].generation };
}

template <typename AssetType>
	requires std::is_move_constructible_v<AssetType> && std::is_move_assignable_v<AssetType>
void AssetPool<AssetType>::Remove(AssetId id)
{
	std::uint32_t index = id.GetIndex();

	if (index >= m_meta.size())
		return;
	if (!m_meta[index].is_active)
		return;
	if (m_meta[index].generation != id.GetGeneration())
		return;

	m_meta[index].is_active = 0;
	m_meta[index].generation++;

	// slots are retired once they reach the maximum generation to prevent collisions with old handles
	if (m_meta[index].generation < GenerationMask)
		m_free_indices.push_back(index);
}

template <typename AssetType>
	requires std::is_move_constructible_v<AssetType> && std::is_move_assignable_v<AssetType>
AssetType const * AssetPool<AssetType>::Get(AssetId id) const
{
	std::uint32_t index = id.GetIndex();

	if (index >= m_meta.size())
		return nullptr;
	if (!m_meta[index].is_active)
		return nullptr;
	if (m_meta[index].generation != id.GetGeneration())
		return nullptr;

	return &m_assets[index];
}

template <typename AssetType>
	requires std::is_move_constructible_v<AssetType> && std::is_move_assignable_v<AssetType>
AssetType * AssetPool<AssetType>::Get(AssetId id)
{
	std::uint32_t index = id.GetIndex();

	if (index >= m_meta.size())
		return nullptr;
	if (!m_meta[index].is_active)
		return nullptr;
	if (m_meta[index].generation != id.GetGeneration())
		return nullptr;

	return &m_assets[index];
}
