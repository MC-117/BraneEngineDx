#pragma once

#include "RenderInterface.h"
#include "../GPUBuffer.h"
#include "../SkeletonMesh.h"
#include "MaterialRenderData.h"
#include "LightRenderData.h"

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
	unsigned int staticTotalTransformIndexCount = 0;
	bool willStaticUpdate = false;
	bool staticUpdate = true;

	MeshTransformData meshTransformData;
	map<Guid, MeshTransformIndex> meshTransformIndex;

	MeshTransformData staticMeshTransformData;
	map<Guid, MeshTransformIndex> staticMeshTransformIndex;

	GPUBuffer transformBuffer = GPUBuffer(GB_Struct, 16 * sizeof(float));
	GPUBuffer transformIndexBuffer = GPUBuffer(GB_Vertex, sizeof(InstanceDrawData));

	void setUpdateStatic();

	unsigned int setMeshTransform(const Matrix4f& transformMat);
	unsigned int setMeshTransform(const vector<Matrix4f>& transformMats);
	MeshTransformIndex* getMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	MeshTransformIndex* setMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex);

	unsigned int setStaticMeshTransform(const Matrix4f& transformMat);
	unsigned int setStaticMeshTransform(const vector<Matrix4f>& transformMats);
	MeshTransformIndex* getStaticMeshPartTransform(MeshPart* meshPart, Material* material);
	MeshTransformIndex* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, unsigned int transformIndex);
	MeshTransformIndex* setStaticMeshPartTransform(MeshPart* meshPart, Material* material, MeshTransformIndex* transformIndex);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void clean();
	void cleanStatic();
	void cleanStatic(unsigned int base, unsigned int count);
	void cleanPartStatic(MeshPart* meshPart, Material* material);
};

struct MeshRenderCommand : public IRenderCommand
{
	int instanceID = 0;
	int instanceIDCount = 0;
	bool isStatic = false;
	bool isNonTransformIndex = false;
	bool hasShadow = true;
	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode() const;
	virtual bool canCastShadow() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct MeshDataRenderPack : public IRenderPack
{
	MeshTransformRenderData& meshTransformDataPack;
	LightRenderData& lightDataPack;

	MaterialRenderData* materialData;
	map<MeshPart*, MeshTransformIndex*> meshParts;
	vector<DrawElementsIndirectCommand> cmds;

	MeshDataRenderPack(MeshTransformRenderData& meshTransformDataPack, LightRenderData& lightDataPack);

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void setRenderData(MeshPart* part, MeshTransformIndex* data);
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext);
};