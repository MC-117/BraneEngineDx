#pragma once
#ifndef _SKELETONMESHRENDER_H_
#define _SKELETONMESHRENDER_H_

#include "SkeletonMesh.h"
#include "MeshRender.h"
#include "MorphTargetWeight.h"

class SkeletonRenderData;

class ENGINE_API SkeletonMeshRender : public MeshRender
{
public:
	SkeletonMesh* skeletonMesh;
	MorphTargetWeight morphWeights;

	SkeletonMeshRender();
	SkeletonMeshRender(SkeletonMesh& mesh, Material& material);
	virtual ~SkeletonMeshRender();

	virtual void setMesh(Mesh* mesh);
	virtual unsigned int getBoneCount() const;
	virtual void resetBoneTransform();
	virtual void setBoneTransform(unsigned int index, const Matrix4f& mat);

	virtual void fillMaterialsByDefault();
	virtual void render(RenderInfo& info);
	virtual vector<Matrix4f>& getTransformMatrixs();

	SkeletonRenderData* getRenderData();

	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	SkeletonRenderData* renderData = NULL;
};

#endif // !_SKELETONMESHRENDER_H_
