#pragma once

#include "RenderInterface.h"
#include "../GPUBuffer.h"

struct MeshTransformData
{
	vector<Matrix4f> transforms;
	unsigned int batchCount = 0;

	unsigned int setMeshTransform(const Matrix4f& transformMat);
	unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
	bool updataMeshTransform(const Matrix4f& transformMat, unsigned int base);
	bool updataMeshTransform(const vector<Matrix4f>& transformMats, unsigned int base);
	void clean();
	bool clean(unsigned int base, unsigned int count);
};

struct MeshTransformIndex
{
	vector<InstanceDrawData> indices;
	unsigned int batchCount = 0;
	unsigned int indexBase = 0;
};

struct MeshTransformRenderData : public IRenderData
{
	unsigned int totalTransformIndexCount = 0;
	bool frequentUpdate = true;
	bool delayUpdate = false;
	bool needUpdate = true;

	MeshTransformData meshTransformData;
	map<Guid, MeshTransformIndex> meshTransformIndex;

	GPUBuffer transformBuffer = GPUBuffer(GB_Storage, GBF_Struct, 16 * sizeof(float));
	GPUBuffer transformIndexBuffer = GPUBuffer(GB_Vertex, GBF_Struct, sizeof(InstanceDrawData));

	void setFrequentUpdate(bool value);
	void setDelayUpdate();
	bool getNeedUpdate() const;

	unsigned int setMeshTransform(const Matrix4f& transformMat);
	unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
	MeshTransformIndex* getMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex, unsigned int transformCount = 1);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex);

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
	void cleanTransform(unsigned int base, unsigned int count);
	void cleanPart(MeshPart* meshPart, Material* material);
};