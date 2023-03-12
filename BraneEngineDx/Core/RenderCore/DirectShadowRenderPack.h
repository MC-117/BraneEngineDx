#pragma once

#include "MeshRenderPack.h"

struct DirectShadowRenderCommand : public IRenderCommand
{
	int instanceID = 0;
	int instanceIDCount = 0;
	MeshTransformIndex* transformIndex = NULL;
	MainLightData* mainLightData = NULL;
	virtual bool isValid() const;
	virtual Enum<ShaderFeature> getShaderFeature() const;
	virtual RenderMode getRenderMode() const;
	virtual bool canCastShadow() const;
	virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct DirectShadowRenderPack : public IRenderPack
{
	MeshTransformRenderData& meshTransformDataPack;

	MaterialRenderData* materialData;
	map<MeshPart*, MeshTransformIndex*> meshParts;
	vector<DrawElementsIndirectCommand> cmds;

	DirectShadowRenderPack(MeshTransformRenderData& meshTransformDataPack);

	virtual bool setRenderCommand(const IRenderCommand& command);
	virtual void setRenderData(MeshPart* part, MeshTransformIndex* data);
	virtual void excute(IRenderContext& context, RenderTaskContext& taskContext);
};