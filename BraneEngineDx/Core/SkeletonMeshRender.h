#pragma once
#ifndef _SKELETONMESHRENDER_H_
#define _SKELETONMESHRENDER_H_

#include "SkeletonMesh.h"
#include "MeshRender.h"
#include "MorphTargetWeight.h"

class SkeletonMeshRender : public MeshRender
{
public:
	SkeletonMesh* skeletonMesh;
	vector<int> boneRemapIndex;
	vector<Matrix4f> transformMats;
	MorphTargetWeight morphWeights;

	SkeletonMeshRender();
	SkeletonMeshRender(SkeletonMesh& mesh, Material& material);
	virtual ~SkeletonMeshRender();

	virtual void setMesh(Mesh* mesh);

	virtual void fillMaterialsByDefault();
	virtual void render(RenderInfo& info);
	virtual vector<Matrix4f>& getTransformMatrixs();

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
};

#endif // !_SKELETONMESHRENDER_H_
