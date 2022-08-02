#pragma once
#ifndef _SKELETON_H_
#define _SKELETON_H_

#include "BoneConstraint.h"
#include "../SkeletonMesh.h"

class Skeleton
{
public:
	struct BoneInfo
	{
		int index;
		int parentIndex;
		Bone* bone = NULL;
		BoneData* data = NULL;
		set<int> sharedSkeletonIndices;
	};

	struct SkeletonInfo
	{
		int index;
		SkeletonData* data;
		vector<int> boneRemapIndex;
	};

	map<string, int> boneNames;
	vector<BoneInfo*> bones;
	vector<SkeletonInfo*> skeletons;
	vector<BoneConstraint*> constraints;

	Skeleton() = default;
	~Skeleton();

	BoneData* validateSkeletonData(SkeletonData& data);
	bool addSkeletonData(SkeletonData& data);
	bool removeSkeletonData(int index);
	BoneInfo* getBone(const string& name);
	BoneInfo* getBone(int index);
	SkeletonInfo* getSkeleton(int index);

	void setReferencePose();

	void addBoneConstraint(BoneConstraint* constraint);

	void solveConstraint(float deltaTime);

	void destroy(bool applyToChild);
};

#endif // !_SKELETON_H_