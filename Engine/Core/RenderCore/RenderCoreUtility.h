#pragma once

#include "RenderInterface.h"
#include "../PipelineState.h"

class IMaterial;

ENGINE_API CullType getMaterialCullMode(IMaterial* material, bool reverseCullMode);
ENGINE_API CullType getMaterialCullMode(MaterialRenderData* material, bool reverseCullMode);
void bindMaterial(IRenderContext& context, IMaterial* material);

void updateRenderDataMainThread(IUpdateableRenderData* renderData, long long mainFrame);
void updateRenderDataRenderThread(IUpdateableRenderData* renderData, long long renderFrame);

void collectRenderData(RenderGraph* graph, IUpdateableRenderData& data);
void collectRenderDataInCommand(RenderGraph* graph, IRenderCommand& command);

ENGINE_API void setupPSODescFromRenderTarget(GraphicsPipelineStateDesc& desc, const RenderTarget& renderTarget);
ENGINE_API void setupPSODescFromRenderTask(GraphicsPipelineStateDesc& desc, const RenderTask& task, CullType cullType);

ENGINE_API GraphicsPipelineState* fetchPSOIfDescChangedThenInit(IPipelineState* pipelineState, const GraphicsPipelineStateDesc& desc);
ENGINE_API ComputePipelineState* fetchPSOIfDescChangedThenInit(IPipelineState* pipelineState, const ComputePipelineStateDesc& desc);
