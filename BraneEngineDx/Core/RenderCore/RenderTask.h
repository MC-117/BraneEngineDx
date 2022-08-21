#pragma once

#include "RenderInterface.h"
#include "CameraData.h"
#include "../Utility/hash.h"

#include "../Utility/Utility.h"
#include "../SkeletonMesh.h"
#include "../GPUBuffer.h"
#include "../RenderTarget.h"

class Camera;

struct CameraRenderData : public IRenderData
{
	Camera* camera;
	CameraData data;
	Color clearColor;
	int renderOrder;
	RenderTarget* renderTarget;
	GPUBuffer buffer = GPUBuffer(GB_Constant, sizeof(CameraData));

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};

struct MaterialRenderData : public IRenderData
{
	Material* material;
	ShaderProgram* program;
	MaterialDesc desc;
	IMaterial* vendorMaterial;

	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
};

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

struct MeshDataRenderPack : public IRenderPack
{
	map<MeshPart*, MeshTransformIndex*> meshParts;
	vector<DrawElementsIndirectCommand> cmds;

	void setRenderData(MeshPart* part, MeshTransformIndex* data);
	virtual void excute(IRenderContext& context);
};

struct RenderTask
{
	struct Hasher
	{
		size_t operator()(const RenderTask& t) const;
		size_t operator()(const RenderTask* t) const;
	};

	struct ExecutionOrder
	{
		bool operator()(const RenderTask& t0, const RenderTask& t1) const;
		bool operator()(const RenderTask* t0, const RenderTask* t1) const;
	};
	size_t hashCode = 0;
	int age = 0;
	CameraRenderData* cameraData = NULL;
	ShaderProgram* shaderProgram = NULL;
	MaterialRenderData* materialData = NULL;
	MeshData* meshData = NULL;
	list<IRenderData*> extraData;
	IRenderPack* renderPack = NULL;
};