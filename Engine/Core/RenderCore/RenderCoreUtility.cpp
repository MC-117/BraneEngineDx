#include "RenderCoreUtility.h"
#include "../IRenderContext.h"

void bindMaterialCullMode(IRenderContext& context, IMaterial* material, bool reverseCullMode)
{
    if (material->desc.isTwoSide)
        context.setCullState(Cull_Off);
    else if (material->desc.cullFront)
        context.setCullState(reverseCullMode ? Cull_Back : Cull_Front);
    else
        context.setCullState(reverseCullMode ? Cull_Front : Cull_Back);
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
