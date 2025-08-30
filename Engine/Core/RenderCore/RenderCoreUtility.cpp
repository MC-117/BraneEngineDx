#include "RenderCoreUtility.h"

#include "RenderTask.h"
#include "../IRenderContext.h"
#include "../RenderTarget.h"

CullType getMaterialCullMode(IMaterial* material, bool reverseCullMode)
{
    if (material) {
        if (material->desc.isTwoSide)
            return Cull_Off;
        if (material->desc.cullFront)
            return reverseCullMode ? Cull_Back : Cull_Front;
    }
    return reverseCullMode ? Cull_Front : Cull_Back;
}

CullType getMaterialCullMode(MaterialRenderData* material, bool reverseCullMode)
{
    if (material) {
        if (material->desc.isTwoSide)
            return Cull_Off;
        if (material->desc.cullFront)
            return reverseCullMode ? Cull_Back : Cull_Front;
    }
    return reverseCullMode ? Cull_Front : Cull_Back;
}

void bindMaterial(IRenderContext& context, IMaterial* material)
{
    context.bindMaterialBuffer(material);
    context.bindMaterialTextures(material);
    context.bindMaterialImages(material);
}

void updateRenderDataMainThread(IUpdateableRenderData* renderData, long long mainFrame)
{
    if (renderData && renderData->usedFrameMainThread < mainFrame) {
        renderData->updateMainThread();
        renderData->usedFrameMainThread = mainFrame;
    }
}

void updateRenderDataRenderThread(IUpdateableRenderData* renderData, long long renderFrame)
{
    if (renderData && renderData->usedFrameRenderThread < renderFrame) {
        renderData->updateRenderThread();
        renderData->usedFrameRenderThread = renderFrame;
    }
}

void collectRenderData(RenderGraph* graph, IUpdateableRenderData& data)
{
    graph->getRenderDataCollectorMainThread()->add(data);
    graph->getRenderDataCollectorRenderThread()->add(data);
}

void collectRenderDataInCommand(RenderGraph* graph, IRenderCommand& command)
{
    command.collectRenderData(graph->getRenderDataCollectorMainThread(), graph->getRenderDataCollectorRenderThread());
}

void setupPSODescFromRenderTarget(GraphicsPipelineStateDesc& desc, const RenderTarget& renderTarget)
{
    desc.dsvFormat = renderTarget.getDepthTextureFormat();
    desc.renderTargetCount = renderTarget.getTextureCount();
    for (int index = 0; index < desc.renderTargetCount; index++) {
        desc.rtvFormats[index] = renderTarget.getTextureFormat(index);
    }
}

void setupPSODescFromRenderTask(GraphicsPipelineStateDesc& desc, const RenderTask& task, CullType cullType)
{
    desc.program = task.shaderProgram;
    desc.meshType = task.meshData->type;
    desc.renderMode = task.renderMode;
    desc.cullType = cullType;
    setupPSODescFromRenderTarget(desc, *task.surface.renderTarget);
}

GraphicsPipelineState* fetchPSOIfDescChangedThenInit(IPipelineState* pipelineState, const GraphicsPipelineStateDesc& desc)
{
    GraphicsPipelineState* graphicsPSO = dynamic_cast<GraphicsPipelineState*>(pipelineState);
    if (graphicsPSO == NULL || graphicsPSO->desc.id != desc.id) {
        graphicsPSO = VendorManager::getInstance().getVendor().fetchGraphicsPipelineState(desc);
    }
    if (graphicsPSO) {
        graphicsPSO->init();
    }
    return graphicsPSO;
}

ComputePipelineState* fetchPSOIfDescChangedThenInit(IPipelineState* pipelineState, const ComputePipelineStateDesc& desc)
{
    ComputePipelineState* computePSO = dynamic_cast<ComputePipelineState*>(pipelineState);
    if (computePSO == NULL || computePSO->desc.id != desc.id) {
        computePSO = VendorManager::getInstance().getVendor().fetchComputePipelineState(desc);
    }
    if (computePSO) {
        computePSO->init();
    }
    return computePSO;
}
