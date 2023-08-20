#pragma once
#ifndef _SKELETONMESHDATA_H_
#define _SKELETONMESHDATA_H_

#include "MeshData.h"

class MorphMeshData
{
public:
	// Data structure: |     vertex 0      |     vertex 1      |
	//                 | morph 0 | morph 1 | morph 0 | morph 1 |
	//                 | op | on | op | on | op | on | op | on |
	// op: offsetPosition	on: offsetNormal
	vector<Vector4f> verticesAndNormals;
	unsigned int vertexCount = 0;
	unsigned int morphCount = 0;

	MorphMeshData();
	MorphMeshData(unsigned int vertexCount, unsigned int morphCount);

	void init(unsigned int vertexCount, unsigned int morphCount);
};

class SkeletonMeshData : public MeshData
{
public:
	vector<Vector4u> boneIndexes;
	vector<Vector4f> weights;
	unsigned int boneCount;
	MorphMeshData morphMeshData;

	SkeletonMeshData();

	//virtual void updateMorphWeights(vector<float>& weights);
};

struct SkeletonMeshPartDesc
{
	SkeletonMeshData* meshData = NULL;
	unsigned int vertexFirst = 0;
	unsigned int vertexCount = 0;
	unsigned int elementFirst = 0;
	unsigned int elementCount = 0;
	MorphMeshData* morphMeshData = NULL;
};

#endif // !_SKELETONMESHDATA_H_
