#pragma once
#include "../../Core/PipelineState.h"

#ifdef VENDOR_USE_DX11

#include "DX11.h"
#include "DX11ShaderStage.h"

class DX11GraphicsPipelineState : public GraphicsPipelineState
{
public:
    DX11Context& dxContext;

    D3D_PRIMITIVE_TOPOLOGY dx11PrimitiveTopology;
    ComPtr<ID3D11RasterizerState> dx11RasterizerState;
    ComPtr<ID3D11BlendState> dx11BlendState;
    ComPtr<ID3D11DepthStencilState> dx11DepthStencilState;
    ComPtr<ID3D11InputLayout> dx11InputLayout;
    DX11ShaderProgram* dx11ShaderProgram = NULL;

    DX11GraphicsPipelineState(DX11Context& context, const GraphicsPipelineStateDesc& desc);

    virtual bool isValid() const;

    virtual bool init();
};

class DX11ComputePipelineState : public ComputePipelineState
{
public:
    DX11Context& dxContext;

    DX11ShaderProgram* dx11ShaderProgram = NULL;

    DX11ComputePipelineState(DX11Context& context, const ComputePipelineStateDesc& desc);

    virtual bool isValid() const;

    virtual bool init();
};

#endif