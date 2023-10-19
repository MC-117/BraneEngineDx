#pragma once

#include "RenderInterface.h"
#include "../GPUBuffer.h"
#include "../SkeletonMesh.h"
#include "MaterialRenderData.h"
#include "LightRenderData.h"
#include "MeshBatchRenderData.h"
#include "ViewCullingContext.h"

struct MeshRenderCommand : public IRenderCommand
{
	int instanceID = 0;
	int instanceIDCount = 0;
	bool hasShadow = true;
	bool hasPreDepth = false;
	bool reverseCullMode = false;
	MeshBatchDrawCall* meshBatchDrawCall = NULL;

	MeshBatchDrawKey getMeshBatchDrawKey() const;

	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode() const;
	virtual bool canCastShadow() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct MeshDataRenderPack : public IRenderPack
{
	LightRenderData& lightDataPack;

	MaterialRenderData* materialData;
	unordered_set<MeshBatchDrawCall*> meshBatchDrawCalls;

	MeshDataRenderPack(LightRenderData& lightDataPack);

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext);
	virtual void reset();
};