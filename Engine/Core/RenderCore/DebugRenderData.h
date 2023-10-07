#pragma once

#include "RenderInterface.h"
#include "../GPUBuffer.h"

struct CameraRenderData;

struct DebugRenderData : public IRenderData
{
    int maxSize;
    vector<LineDrawData> updateLineData;
    GPUBuffer uploadLineBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(Vector4f) * 2);
    GPUBuffer lineBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(Vector4f) * 2, GAF_ReadWrite, CAF_None);
    GPUBuffer flagBufferA = GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None);
    GPUBuffer flagBufferB = GPUBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None);
    GPUBuffer commandBuffer = GPUBuffer(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None);

    GPUBuffer* flagBuffer = NULL;
    GPUBuffer* lastFlagBuffer = NULL;

    DebugRenderData();

    virtual void create();
    virtual void release();
    virtual void upload();
    virtual void bind(IRenderContext& context);
    virtual void clean();
    virtual void bindForRender(IRenderContext& context);
    virtual void unbind(IRenderContext& context);

    void triggerPersistentDebugDraw();
    bool isTriggerPersistentDebugDraw() const;

    void initBuffer(IRenderContext& context, bool forceClearAll);
    void debugDraw(IRenderContext& context, CameraRenderData& cameraRenderData);

protected:
    bool firstTick = true;
    bool persistentDebugDraw = false;
    static bool isInit;
    static Material* packFreeMaterial;
    static ShaderProgram* packFreeProgram;
    static Material* initDrawArgsMaterial;
    static ShaderProgram* initDrawArgsProgram;
    static Material* drawLineMaterial;
    static ShaderProgram* drawLineProgram;
    static ShaderProgram* drawUploadLineProgram;

    static void loadDefaultResource();
};
