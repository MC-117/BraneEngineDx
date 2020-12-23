#pragma once
#ifndef _SKELETONMESH_H_

#include "Unit.h"

class SkeletonData;

struct BoneData
{
	string name;
	unsigned int index;
	unsigned int siblingIndex;
	Matrix4f offsetMatrix;
	Matrix4f transformMatrix;
	SkeletonData* skeletonData;
	BoneData* parent;
	vector<BoneData*> children;

	unsigned int addChild(const string& name, const Matrix4f& offsetMatrix, const Matrix4f& transformMatrix);
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

	BoneData* getBoneData(const string& name);
	BoneData* getBoneData(unsigned int index);
};

#endif // !_SKELETONMESH_H_