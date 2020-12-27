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
	morphWeightBuffer.resize(morphCount);
	verticesAndNormals.resize(vertexCount * morphCount * 2);
}

void MorphMeshData::bindBase(unsigned int meshId, unsigned int weightId)
{
	if (verticesAndNormalBuffer.empty()) {
		unsigned int offset = sizeof(float) * 4;
		float morphCountV[] = { morphCount, morphCount, morphCount, 1.0 };
		unsigned int size = sizeof(float) * 4 * verticesAndNormals.size();
		verticesAndNormalBuffer.resize(offset + size);
		verticesAndNormalBuffer.uploadSubData(0, 1, morphCountV);
		verticesAndNormalBuffer.uploadSubData(1, verticesAndNormals.size(), verticesAndNormals.data()->data());
	}
	verticesAndNormalBuffer.bindBase(meshId);
	morphWeightBuffer.bindBase(weightId);
}

SkeletonMeshData::SkeletonMeshData()
{
	type = MT_SkeletonMesh;
}

void SkeletonMeshData::updateMorphWeights(vector<float>& weights)
{
	morphMeshData.morphWeightBuffer.uploadData(weights.size(), weights.data());
}
