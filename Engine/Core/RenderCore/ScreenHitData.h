#pragma once

#include "MeshBatchRenderData.h"
#include "CameraData.h"
#include "MaterialRenderData.h"

struct ENGINE_API ScreenHitData : public IRenderData
{
	unsigned int triggerFrame = 0;
	unsigned int hitFrame = 0;
	ScreenHitInfo hitInfo;
	Texture* depthTexture = NULL;
	RenderTarget* renderTarget = NULL;
	GPUBuffer hitInfoBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(ScreenHitInfo), GAF_ReadWrite, CAF_None);
	GPUBuffer readBackBuffer = GPUBuffer(GB_Constant, GBF_Struct, sizeof(ScreenHitInfo), GAF_ReadWrite, CAF_Read);

	void resize(unsigned int width, unsigned int height);
	virtual void create();
	virtual void release();
	virtual void upload();
	virtual void bind(IRenderContext& context);
	void readBack(IRenderContext& context);
};

struct ENGINE_API ScreenHitRenderCommand : public IRenderCommand
{
	int instanceID = 0;
	int instanceIDCount = 0;
	bool reverseCullMode = false;
	MeshBatchDrawCall* meshBatchDrawCall = NULL;
	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual CullType getCullType() const;
	virtual RenderMode getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const;
	virtual bool canCastShadow() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct ScreenHitRenderPack : public IRenderPack
{
	MaterialRenderData* materialData;
	unordered_set<MeshBatchDrawCall*> meshBatchDrawCalls;

	ScreenHitRenderPack();

	virtual bool setRenderCommand(const IRenderCommand& command, const RenderTask& task);
	virtual void excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext);
	virtual void reset();
};