#include "ClusterBuilder.h"
#include "../Mesh.h"

size_t EdgeData::Hash::operator()(const EdgeData& data) const
{
	uint32_t Hash0 = HashPosition(data.pos0);
	uint32_t Hash1 = HashPosition(data.pos1);
	return Murmur32({ Hash0, Hash1 });
}

bool EdgeData::operator==(const EdgeData& other) const
{
	return pos0 == other.pos0 &&
		pos1 == other.pos1 &&
		index == index;
}

bool EdgeData::match(const EdgeData& other) const
{
	return pos0 == other.pos0 &&
		pos1 == other.pos1;
}

EdgeHash::EdgeHash(uint32_t size)
{
	edgeTable.reserve(size);
}

Adjacency::Adjacency(int32_t size) : direct(size)
{
}

void Adjacency::link(int32_t edgeIndex0, int32_t edgeIndex1)
{
	if (direct[edgeIndex0] < 0 &&
		direct[edgeIndex1] < 0) {
		direct[edgeIndex0] = edgeIndex1;
		direct[edgeIndex1] = edgeIndex0;
	}
	else {
		insertUnique(extended, edgeIndex0, edgeIndex1);
		insertUnique(extended, edgeIndex1, edgeIndex0);
	}
}

DisjointSet::DisjointSet(uint32_t size)
	: parents(size)
{
	for (uint32_t i = 0; i < size; i++)
		parents[i] = i;
}

void DisjointSet::addUnion(uint32_t x, uint32_t y)
{
}

void DisjointSet::addUnionSequential(uint32_t x, uint32_t y)
{
	if (x >= y)
		throw runtime_error("");
	if (x == parents[x])
		throw runtime_error("");
	uint32_t px = x;
	uint32_t py = parents[y];
	while (px != py) {
		parents[y] = px;
		if (y == py)
			return;
		y = py;
		py = parents[y];
	}
}

bool ClusterBuilder::clusterTriangles(const MeshPart& mesh)
{
	if (mesh.vertexPerFace != 3)
		return false;
	uint32_t triangleCount = mesh.elementCount;
	uint32_t indicesCount = mesh.vertexPerFace * mesh.elementCount;
	Adjacency adjacency(indicesCount);
	EdgeHash edgeHash(indicesCount);

	auto getPosition = [&](uint32_t index) {
		return mesh.vertex(index);
	};

	for (int edgeIndex = 0; edgeIndex < indicesCount; edgeIndex++) {
		edgeHash.add(edgeIndex, getPosition);
	}

	for (int edgeIndex = 0; edgeIndex < indicesCount; edgeIndex++) {
		int32_t adjIndex = -1;
		int32_t adjCount = 0;
		edgeHash.forAllMatch(edgeIndex, getPosition,
			[&](int32_t edgeIndex, int32_t OtherEdgeIndex)
			{
				adjIndex = OtherEdgeIndex;
				adjCount++;
			});

		if (adjCount > 1)
			adjIndex = -2;

		adjacency.direct[edgeIndex] = adjIndex;
	}

	DisjointSet disjointSet(indicesCount);

	for (uint32_t edgeIndex = 0; edgeIndex < indicesCount; edgeIndex++)
	{
		if (adjacency.direct[edgeIndex] == -2)
		{
			// EdgeHash is built in parallel, so we need to sort before use to ensure determinism.
			// This path is only executed in the rare event that an edge is shared by more than two triangles,
			// so performance impact should be negligible in practice.
			vector<pair<int32_t, int32_t>> edges;
			edgeHash.forAllMatch(edgeIndex, getPosition,
				[&](int32_t edgeIndex0, int32_t edgeIndex1)
				{
					edges.push_back(make_pair(edgeIndex0, edgeIndex1));
				});
			sort(edges.begin(), edges.end());

			for (const pair<int32_t, int32_t>& edge : edges)
			{
				adjacency.link(edge.first, edge.second);
			}
		}

		adjacency.forAll(edgeIndex,
			[&](int32_t edgeIndex0, int32_t edgeIndex1)
			{
				if (edgeIndex0 > edgeIndex1)
					disjointSet.addUnionSequential(edgeIndex0 / 3, edgeIndex1 / 3);
			});
	}

	return true;
}
