#pragma once

#include "MeshRenderPack.h"

struct DirectShadowRenderCommand : public IRenderCommand
{
	int instanceID = 0;
	int instanceIDCount = 0;
	bool reverseCullMode = false;
	MeshBatchDrawCall* meshBatchDrawCall = NULL;
	MainLightData* mainLightData = NULL;
	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode() const;
	virtual bool canCastShadow() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct DirectShadowRenderPack : public IRenderPack
{
	MaterialRenderData* materialData;
	unordered_set<MeshBatchDrawCall*> meshBatchDrawCalls;

	DirectShadowRenderPack();

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext);
	virtual void reset();
};