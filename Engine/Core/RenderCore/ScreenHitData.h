#pragma once

#include "TransformRenderData.h"
#include "CameraData.h"
#include "MaterialRenderData.h"

struct ScreenHitData : public IRenderData
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

struct ScreenHitRenderCommand : public IRenderCommand
{
	int instanceID = 0;
	int instanceIDCount = 0;
	MeshTransformIndex* transformIndex = NULL;
	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode() const;
	virtual bool canCastShadow() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct ScreenHitRenderPack : public IRenderPack
{
	MaterialRenderData* materialData;
	map<MeshPart*, MeshTransformIndex*> meshParts;
	vector<DrawElementsIndirectCommand> cmds;

	ScreenHitRenderPack();

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void setRenderData(MeshPart* part, MeshTransformIndex* data);
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext);
};