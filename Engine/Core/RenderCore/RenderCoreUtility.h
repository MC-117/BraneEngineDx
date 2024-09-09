#pragma once

#include "RenderInterface.h"

class IMaterial;

void bindMaterialCullMode(IRenderContext& context, IMaterial* material, bool reverseCullMode);
void bindMaterial(IRenderContext& context, IMaterial* material);

void updateRenderDataMainThread(IUpdateableRenderData* renderData, long long mainFrame);
void updateRenderDataRenderThread(IUpdateableRenderData* renderData, long long renderFrame);

void collectRenderData(RenderGraph* graph, IUpdateableRenderData& data);
void collectRenderDataInCommand(RenderGraph* graph, IRenderCommand& command);
