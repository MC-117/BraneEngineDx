#include "PipelineState.h"

#include "RenderCore/RenderCoreUtility.h"
#include "Utility/hash.h"

bool GraphicsPipelineStateId::operator==(const GraphicsPipelineStateId& id) const
{
    return memcmp(packedValue, id.packedValue, sizeof(packedValue)) == 0;
}

bool GraphicsPipelineStateId::operator!=(const GraphicsPipelineStateId& id) const
{
    return memcmp(packedValue, id.packedValue, sizeof(packedValue)) != 0;
}

bool GraphicsPipelineStateId::operator<(const GraphicsPipelineStateId& id) const
{
    for (int i = 0; i < NumBit64; i++) {
        if (packedValue[i] < id.packedValue[i]) {
            return true;
        }
        if (packedValue[i] > id.packedValue[i]) {
            return false;
        }
    }
    return false;
}

std::size_t hash<GraphicsPipelineStateId>::operator()(const GraphicsPipelineStateId& id) const noexcept
{
    size_t hash = id.packedValue[0];
    for (int i = 1; i < GraphicsPipelineStateId::NumBit64; i++) {
        hash_combine(hash, id.packedValue[i]);
    }
    return hash;
}

GraphicsPipelineStateDesc::GraphicsPipelineStateDesc()
    : renderMode()
    , program(NULL)
    , flags(0)
    , dsvFormat(TIT_D32_F_S8_UI)
    , meshType(MT_Mesh)
    , cullType(Cull_Back)
    , renderTargetCount(0)
    , sampleCount(1)
    , pad(0)
{
    for (auto& format : rtvFormats) {
        format = TIT_Default;
    }
}

GraphicsPipelineStateDesc::GraphicsPipelineStateDesc(const GraphicsPipelineStateDesc& desc)
{
    id = desc.id;
}

GraphicsPipelineStateDesc::GraphicsPipelineStateDesc(GraphicsPipelineStateDesc&& desc)
{
    id = desc.id;
}

GraphicsPipelineStateDesc GraphicsPipelineStateDesc::forScreen(ShaderProgram* shaderProgram,
    const RenderTarget* renderTarget, BlendMode blendMode)
{
    GraphicsPipelineStateDesc desc;
    if (renderTarget) {
        setupPSODescFromRenderTarget(desc, *renderTarget);
    }
    desc.program = shaderProgram;
    desc.cullType = Cull_Back;
    desc.meshType = MT_Screen;
    desc.renderMode = RenderMode(RS_Post, blendMode);
    return desc;
}

GraphicsPipelineStateDesc GraphicsPipelineStateDesc::forLines(ShaderProgram* shaderProgram,
    const RenderTarget* renderTarget, DepthStencilMode depthStencilMode)
{
    GraphicsPipelineStateDesc desc;
    if (renderTarget) {
        setupPSODescFromRenderTarget(desc, *renderTarget);
    }
    desc.program = shaderProgram;
    desc.cullType = Cull_Back;
    desc.meshType = MT_Lines;
    desc.renderMode = RenderMode(RS_Opaque);
    desc.renderMode.setDepthStencilMode(depthStencilMode);
    return desc;
}

ComputePipelineStateDesc::ComputePipelineStateDesc(ShaderProgram* shaderProgram)
    : program(shaderProgram)
{
}

ComputePipelineStateDesc::ComputePipelineStateDesc(const ComputePipelineStateDesc& desc)
{
    id = desc.id;
}

ComputePipelineStateDesc::ComputePipelineStateDesc(ComputePipelineStateDesc&& desc)
{
    id = desc.id;
}
