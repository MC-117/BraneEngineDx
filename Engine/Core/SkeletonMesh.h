#pragma once
#ifndef _SKELETONMESH_H_
#define _SKELETONMESH_H_

#include "SkeletonMeshData.h"
#include "Mesh.h"

struct SkeletonMeshPart : public MeshPart
{
	MorphMeshData* morphMeshData = NULL;

	SkeletonMeshPart();
	SkeletonMeshPart(const SkeletonMeshPartDesc& desc);
	SkeletonMeshPart(SkeletonMeshData* meshData, unsigned int vertexFirst, unsigned int vertexCount,
		unsigned int indexFirst, unsigned int indexCount, MorphMeshData* morphData = NULL);

	inline Vector4u& boneIndex(unsigned int index) {
		if (index >= vertexCount)
			throw overflow_error("SkeletonMeshPart boneIndex access overflow");
		return ((SkeletonMeshData*)meshData)->boneIndexes[vertexFirst + index];
	}
	inline Vector4f& weight(unsigned int index) {
		if (index >= vertexCount)
			throw overflow_error("SkeletonMeshPart weight access overflow");
		return ((SkeletonMeshData*)meshData)->weights[vertexFirst + index];
	}
	inline Vector4f& morphVertex(unsigned int vertexIndex, unsigned int morphIndex) {
		if (morphMeshData == NULL)
			throw overflow_error("No MorphMeshData");
		if (morphIndex >= morphMeshData->morphCount || (vertexIndex + vertexFirst) >= morphMeshData->vertexCount)
			throw overflow_error("MorphMeshData vertex access overflow");
		return morphMeshData->verticesAndNormals[((vertexIndex + vertexFirst) * morphMeshData->morphCount + morphIndex) * 2];
	}

	inline Vector4f& morphNormal(unsigned int normalIndex, unsigned int morphIndex) {
		if (morphMeshData == NULL)
			throw overflow_error("No MorphMeshData");
		if (morphIndex >= morphMeshData->morphCount || (normalIndex + vertexFirst) >= morphMeshData->vertexCount)
			throw overflow_error("MorphMeshData normal access overflow");
		return morphMeshData->verticesAndNormals[((normalIndex + vertexFirst) * morphMeshData->morphCount + morphIndex) * 2 + 1];
	}
	operator MeshPart();
};

class SkeletonData;

struct BoneData
{
	string name;
	unsigned int index;
	unsigned int siblingIndex;
	Matrix4f offsetMatrix;
	Matrix4f transformMatrix;
	SkeletonData* skeletonData = NULL;
	BoneData* parent = NULL;
	vector<BoneData*> children;

	unsigned int addChild(const string& name, const Matrix4f& offsetMatrix, const Matrix4f & transformMatrix);
	BoneData* getParent();
	void setParent(BoneData* parent);
	BoneData* getChild(unsigned int index = 0);
	BoneData* getSibling(unsigned int index = 0);
	BoneData* getNext(BoneData* limitBone = NULL);
};

class SkeletonData
{
public:
	map<string, unsigned int> boneName;
	vector<BoneData> boneList;
	unsigned int baseBoneOffsetMatIndex;

	BoneData* rootBone = NULL;

	unsigned int getBoneCount() const;
	BoneData* getBoneData(const string& name);
	BoneData* getBoneData(unsigned int index);
};

class SkeletonMesh : public Mesh
{
public:
	SkeletonData skeletonData;
	vector<string> morphName;
	vector<bool> partHasMorph;

	//virtual void updateMorphWeights(vector<float>& weights);
	virtual bool writeObjStream(ostream& os, const vector<int>& partIndex) const;
};

#endif // !_SKELETONMESH_H_
