#pragma once
#include "../Utility/Utility.h"

static FORCEINLINE uint32_t MurmurFinalize32(uint32_t Hash)
{
	Hash ^= Hash >> 16;
	Hash *= 0x85ebca6b;
	Hash ^= Hash >> 13;
	Hash *= 0xc2b2ae35;
	Hash ^= Hash >> 16;
	return Hash;
}

static FORCEINLINE uint32_t Murmur32(std::initializer_list< uint32_t > InitList)
{
	uint32_t Hash = 0;
	for (auto Element : InitList)
	{
		Element *= 0xcc9e2d51;
		Element = (Element << 15) | (Element >> (32 - 15));
		Element *= 0x1b873593;

		Hash ^= Element;
		Hash = (Hash << 13) | (Hash >> (32 - 13));
		Hash = Hash * 5 + 0xe6546b64;
	}

	return MurmurFinalize32(Hash);
}

FORCEINLINE uint32_t HashPosition(const Vector3f& Position)
{
	union { float f; uint32_t i; } x;
	union { float f; uint32_t i; } y;
	union { float f; uint32_t i; } z;

	x.f = Position.x();
	y.f = Position.y();
	z.f = Position.z();

	return Murmur32({
		Position.x() == 0.0f ? 0u : x.i,
		Position.y() == 0.0f ? 0u : y.i,
		Position.z() == 0.0f ? 0u : z.i
		});
}

FORCEINLINE uint32_t Cycle3(uint32_t Value)
{
	uint32_t ValueMod3 = Value % 3;
	uint32_t Value1Mod3 = (1 << ValueMod3) & 3;
	return Value - ValueMod3 + Value1Mod3;
}

template<typename K, typename V>
bool insertUnique(multimap<K, V>& container, const K& k, const V& v)
{
	auto iter = container.equal_range(k);
	bool needInsert = true;
	for (auto b = iter.first; b != iter.second; b++) {
		if (b->second == v) {
			needInsert = false;
			break;
		}
	}
	if (needInsert)
		container.insert(make_pair(k, v));
	return needInsert;
}

struct EdgeData
{
	struct Hash
	{
		size_t operator()(const EdgeData& data) const;
	};

	Vector3f pos0;
	Vector3f pos1;
	uint32_t index = 0;

	bool operator==(const EdgeData& other) const;
	bool match(const EdgeData& other) const;
};

struct EdgeHash
{
	unordered_set<uint32_t> edgeTable;

	EdgeHash(uint32_t size);
	
	template<typename GetPosition>
	void add(uint32_t edgeIndex, GetPosition&& getPosition)
	{
		const Vector3f pos0 = getPosition(edgeIndex);
		const Vector3f pos1 = getPosition(Cycle3(edgeIndex));
		const uint32_t hash0 = HashPosition(pos0);
		const uint32_t hash1 = HashPosition(pos1);
		const uint32_t hash = Murmur32({ hash0, hash1 });
		edgeTable.insert(hash);
	}

	template<typename GetPosition, typename FuncType>
	void forAllMatch(uint32_t edgeIndex, GetPosition&& getPosition, FuncType&& findFunc)
	{
		const Vector3f pos0 = getPosition(edgeIndex);
		const Vector3f pos1 = getPosition(Cycle3(edgeIndex));
		const uint32_t hash0 = HashPosition(pos0);
		const uint32_t hash1 = HashPosition(pos1);
		for (auto index : edgeTable) {

			if (pos0 == getPosition(Cycle3(index)) &&
				pos1 == getPosition(index)) {
				findFunc(edgeIndex, index);
			}
		}
	}
};

struct Adjacency
{
	vector<int32_t> direct;
	multimap<int32_t, int32_t> extended;

	Adjacency(int32_t size);

	void link(int32_t edgeIndex0, int32_t edgeIndex1);

	template< typename FuncType >
	void forAll(int32_t edgeIndex, FuncType&& func) const
	{
		int32_t adjIndex = direct[edgeIndex];
		if (adjIndex != -1)
			func(edgeIndex, adjIndex);
		
		for (auto& iter : extended) {
			func(edgeIndex, iter.second);
		}
	}
};

struct DisjointSet
{
	vector<uint32_t> parents;

	DisjointSet(uint32_t size = 0);

	void addUnion(uint32_t x, uint32_t y);
	void addUnionSequential(uint32_t x, uint32_t y);
};

class MeshPart;

class ClusterBuilder
{
public:
	static bool clusterTriangles(const MeshPart& mesh);
};
