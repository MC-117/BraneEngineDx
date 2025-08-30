#include "DX11PipelineState.h"

const ComPtr<ID3D11InputLayout>& getDX11PrimitiveTopology(DX11Context& context, MeshType type, D3D_PRIMITIVE_TOPOLOGY& topology)
{
    switch (type) {
    case MT_Mesh: topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; return context.meshInputLayout;
    case MT_SkeletonMesh: topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; return context.skeletonMeshInputLayout;
    case MT_Terrain: topology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST; return context.terrainInputLayout;
    case MT_Lines: topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST; return context.screenInputLayout;
    case MT_Screen: topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; return context.screenInputLayout;
    case MT_Particle: topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; return context.screenInputLayout;
    }
    throw runtime_error("Unknown mesh type");
}

const ComPtr<ID3D11RasterizerState>& getDX11RasterizerState(DX11Context& context, CullType type)
{
    switch (type) {
    case Cull_Off: return context.rasterizerCullOff;
    case Cull_Back: return context.rasterizerCullBack;
    case Cull_Front: return context.rasterizerCullFront;
    }
    throw runtime_error("Unknown cull type");
}

DX11GraphicsPipelineState::DX11GraphicsPipelineState(DX11Context& context, const GraphicsPipelineStateDesc& desc)
    : GraphicsPipelineState(desc), dxContext(context)
{
    dx11InputLayout = getDX11PrimitiveTopology(context, desc.meshType, dx11PrimitiveTopology);
    dx11BlendState = context.getOrCreateBlendState(desc.renderMode.getRenderTargetModes());
    dx11DepthStencilState = context.getOrCreateDepthStencilState(desc.renderMode.getDepthStencilMode());
    dx11RasterizerState = getDX11RasterizerState(context, desc.cullType);
    dx11ShaderProgram = dynamic_cast<DX11ShaderProgram*>(desc.program);
}

bool DX11GraphicsPipelineState::isValid() const
{
    return dx11InputLayout && dx11BlendState && dx11DepthStencilState && dx11RasterizerState && dx11ShaderProgram;
}

bool DX11GraphicsPipelineState::init()
{
    assert(isValid());
    
    return dx11ShaderProgram->init();
}

DX11ComputePipelineState::DX11ComputePipelineState(DX11Context& context, const ComputePipelineStateDesc& desc)
    : ComputePipelineState(desc), dxContext(context)
{
    dx11ShaderProgram = dynamic_cast<DX11ShaderProgram*>(desc.program);
}

bool DX11ComputePipelineState::isValid() const
{
    return dx11ShaderProgram;
}

bool DX11ComputePipelineState::init()
{
    assert(isValid());
    
    return dx11ShaderProgram->init();
}
