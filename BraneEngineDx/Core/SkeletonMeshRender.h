#pragma once
#ifndef _SKELETONMESHRENDER_H_
#define _SKELETONMESHRENDER_H_

#include "SkeletonMesh.h"
#include "MeshRender.h"

class SkeletonMeshRender : public MeshRender
{
public:
	SkeletonMesh& skeletonMesh;
	vector<Matrix4f> transformMats;
	vector<float> morphWeights;

	SkeletonMeshRender(SkeletonMesh& mesh, Material& material);
	virtual ~SkeletonMeshRender();
	virtual void fillMaterialsByDefault();
	virtual bool setMorphWeight(unsigned int index, float weight);
	virtual void render(RenderInfo& info);
	virtual vector<Matrix4f>& getTransformMatrixs();
protected:
	bool morphUpdate = true;
};

#endif // !_SKELETONMESHRENDER_H_
