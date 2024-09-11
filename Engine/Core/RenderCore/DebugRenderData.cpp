#include "DebugRenderData.h"

#include "CameraRenderData.h"
#include "../Asset.h"
#include "../Material.h"

bool DebugRenderData::isInit = false;
Material* DebugRenderData::packFreeMaterial = NULL;
ShaderProgram* DebugRenderData::packFreeProgram = NULL;
Material* DebugRenderData::initDrawArgsMaterial = NULL;
ShaderProgram* DebugRenderData::initDrawArgsProgram = NULL;
Material* DebugRenderData::drawLineMaterial = NULL;
ShaderProgram* DebugRenderData::drawLineProgram = NULL;
ShaderProgram* DebugRenderData::drawUploadLineProgram = NULL;

DebugRenderData::DebugRenderData()
    : maxSize(81920)
{
}

void DebugRenderData::create()
{
    loadDefaultResource();
    packFreeProgram->init();
    initDrawArgsProgram->init();
    drawLineProgram->init();
    drawUploadLineProgram->init();
}

void DebugRenderData::release()
{
    uploadLineBuffer.resize(0);
    lineBuffer.resize(0);
    flagBufferA.resize(0);
    flagBufferB.resize(0);
}

void DebugRenderData::upload()
{
    uploadLineBuffer.uploadData(updateLineData.size() * 2, updateLineData.data(), true);
    lineBuffer.resize(maxSize);
    flagBufferA.resize(maxSize + 1);
    flagBufferB.resize(maxSize + 1);
    commandBuffer.resize(sizeof(DrawArraysIndirectCommand) / sizeof(unsigned int));

    if (flagBuffer == NULL || flagBuffer == &flagBufferB) {
        flagBuffer = &flagBufferA;
        lastFlagBuffer = &flagBufferB;
    }
    else {
        flagBuffer = &flagBufferB;
        lastFlagBuffer = &flagBufferA;
    }
}

static const ShaderPropertyName outDebugLinesName = "outDebugLines";
static const ShaderPropertyName outDebugFlagsName = "outDebugFlags";

static const ShaderPropertyName debugLinesName = "debugLines";
static const ShaderPropertyName debugFlagsName = "debugFlags";

void DebugRenderData::bind(IRenderContext& context)
{
    context.bindBufferBase(lineBuffer.getVendorGPUBuffer(), outDebugLinesName, { true });
    context.bindBufferBase(flagBuffer->getVendorGPUBuffer(), outDebugFlagsName, { true });
}

void DebugRenderData::clean()
{
    updateLineData.clear();
    persistentDebugDraw = false;
}

void DebugRenderData::bindForRender(IRenderContext& context)
{
    context.bindBufferBase(lineBuffer.getVendorGPUBuffer(), debugLinesName);
    context.bindBufferBase(flagBuffer->getVendorGPUBuffer(), debugFlagsName);
}

void DebugRenderData::unbind(IRenderContext& context)
{
    context.unbindBufferBase(outDebugLinesName);
    context.unbindBufferBase(outDebugFlagsName);
}

void DebugRenderData::triggerPersistentDebugDraw()
{
    persistentDebugDraw = true;
}

bool DebugRenderData::isTriggerPersistentDebugDraw() const
{
    return persistentDebugDraw;
}

void DebugRenderData::initBuffer(IRenderContext& context, bool forceClearAll)
{
    static const ShaderPropertyName lastDebugFlagsName = "lastDebugFlags";
    
    context.clearOutputBufferUint(flagBuffer->getVendorGPUBuffer(), Vector4u::Zero());
    if (firstTick || persistentDebugDraw || forceClearAll) {
        context.clearOutputBufferUint(lastFlagBuffer->getVendorGPUBuffer(), { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF });
    }

    unsigned int packSize = packFreeMaterial->getLocalSize().x();
    context.bindShaderProgram(packFreeProgram);
    context.bindBufferBase(flagBuffer->getVendorGPUBuffer(), outDebugFlagsName, { true });
    context.bindBufferBase(lastFlagBuffer->getVendorGPUBuffer(), lastDebugFlagsName, { true });
    context.dispatchCompute(ceilf(maxSize* 0.5f / (float)packSize), 1, 1);

    context.unbindBufferBase(outDebugFlagsName);
    context.unbindBufferBase(lastDebugFlagsName);

    firstTick = false;
}

void DebugRenderData::debugDraw(IRenderContext& context, CameraRenderData& cameraRenderData)
{
    static const ShaderPropertyName commandBufferName = "commandBuffer";

    context.bindShaderProgram(initDrawArgsProgram);
    context.bindBufferBase(flagBuffer->getVendorGPUBuffer(), outDebugFlagsName, { true });
    context.bindBufferBase(commandBuffer.getVendorGPUBuffer(), commandBufferName, { true });
    context.dispatchCompute(1, 1, 1);

    context.unbindBufferBase(outDebugFlagsName);
    context.unbindBufferBase(commandBufferName);

    context.bindShaderProgram(drawLineProgram);
    context.bindBufferBase(lineBuffer.getVendorGPUBuffer(), debugLinesName);
    context.bindBufferBase(flagBuffer->getVendorGPUBuffer(), debugFlagsName);
    cameraRenderData.bindCameraBuffOnly(context);
    context.setViewport(0, 0, cameraRenderData.data.viewSize.x(),  cameraRenderData.data.viewSize.y());
    context.setLineDrawContext();
    cameraRenderData.surface.bind(context, Clear_None, Clear_All);
    context.setRenderOpaqueState(DepthStencilMode::DepthTestWritable(), 0);
    context.drawArrayIndirect(commandBuffer.getVendorGPUBuffer(), 0);

    context.unbindBufferBase(debugLinesName);
    context.unbindBufferBase(debugFlagsName);
}

void DebugRenderData::gizmoDraw(IRenderContext& context, CameraRenderData& cameraRenderData)
{
    context.bindShaderProgram(drawUploadLineProgram);
    context.bindBufferBase(uploadLineBuffer.getVendorGPUBuffer(), debugLinesName);
    cameraRenderData.bindCameraBuffOnly(context);
    context.setViewport(0, 0, cameraRenderData.data.viewSize.x(),  cameraRenderData.data.viewSize.y());
    context.setLineDrawContext();
    cameraRenderData.surface.bind(context, Clear_None, Clear_All);
    context.setRenderOpaqueState(DepthStencilMode::DepthTestWritable(), 0);
    DrawArraysIndirectCommand cmd;
    cmd.count = uploadLineBuffer.size();
    cmd.first = 0;
    cmd.baseInstance = 0;
    cmd.instanceCount = 1;
    context.drawArray(cmd);

    context.unbindBufferBase(debugLinesName);
}

void DebugRenderData::loadDefaultResource()
{
    if (isInit)
        return;
    packFreeMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/DebugDraw/DebugDrawPackFree.mat");
    if (packFreeMaterial == NULL)
        throw runtime_error("Not found default shader");
    packFreeProgram = packFreeMaterial->getShader()->getProgram(Shader_Default);
    if (packFreeProgram == NULL)
        throw runtime_error("Not found default shader");
    
    initDrawArgsMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/DebugDraw/DebugDrawInitArgs.mat");
    if (initDrawArgsMaterial == NULL)
        throw runtime_error("Not found default shader");
    initDrawArgsProgram = initDrawArgsMaterial->getShader()->getProgram(Shader_Default);
    if (initDrawArgsProgram == NULL)
        throw runtime_error("Not found default shader");
    
    drawLineMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/DebugDraw/DebugDrawLines.mat");
    if (drawLineMaterial == NULL)
        throw runtime_error("Not found default shader");
    drawLineProgram = drawLineMaterial->getShader()->getProgram(Shader_Default);
    if (drawLineProgram == NULL)
        throw runtime_error("Not found default shader");
    drawUploadLineProgram = drawLineMaterial->getShader()->getProgram(Shader_Custom_1);
    if (drawUploadLineProgram == NULL)
        throw runtime_error("Not found default shader");
    
    isInit = true;
}


