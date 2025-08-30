#pragma once

#include "MeshRenderPack.h"

struct ENGINE_API DecalRenderCommand : public MeshRenderCommand
{
    Enum<DecalMask> decalMask;
    Matrix4f localToDecalClip;
    
    virtual CullType getCullType() const;
    virtual RenderMode getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const;
    virtual IRenderPack* createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const;
};

struct DecalRenderPack : public IRenderPack
{
    MaterialRenderData* materialData;
    Matrix4f localToDecalClip;
    unordered_map<MeshBatchDrawCall*, GraphicsPipelineState*> meshBatchDrawCalls;

    DecalRenderPack();

    virtual bool setRenderCommand(const IRenderCommand& command, const RenderTask& task);
    virtual void excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext);
    virtual void reset();
};
