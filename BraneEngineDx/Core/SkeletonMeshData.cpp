#include "SkeletonMeshData.h"

MorphMeshData::MorphMeshData()
{
}

MorphMeshData::MorphMeshData(unsigned int vertexCount, unsigned int morphCount)
{
	init(vertexCount, morphCount);
}

void MorphMeshData::init(unsigned int vertexCount, unsigned int morphCount)
{
	this->vertexCount = vertexCount;
	this->morphCount = morphCount;
	verticesAndNormals.resize(vertexCount * morphCount * 2);
}

SkeletonMeshData::SkeletonMeshData()
{
	type = MT_SkeletonMesh;
}

void SkeletonMeshData::updateMorphWeights(vector<float>& weights)
{
}
