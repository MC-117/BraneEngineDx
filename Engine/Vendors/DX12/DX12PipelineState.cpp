#include "DX12PipelineState.h"

bool DX12GraphicPipelineState::ID::operator<(const ID& id) const
{
    return word0.value < id.word0.value || word1.value < id.word1.value;
}

DX12GraphicPipelineState::DX12GraphicPipelineState()
{
    reset();
}

DX12GraphicPipelineState::~DX12GraphicPipelineState()
{
    release();
}

void DX12GraphicPipelineState::reset()
{
    desc.SampleMask = UINT_MAX;

    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    desc.RasterizerState.FrontCounterClockwise = TRUE;

    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    desc.BlendState.IndependentBlendEnable = false;
    desc.BlendState.AlphaToCoverageEnable = false;
    auto& rt0 = desc.BlendState.RenderTarget[0];
    rt0.BlendEnable = false;
    rt0.SrcBlend = D3D12_BLEND_ZERO;
    rt0.DestBlend = D3D12_BLEND_ONE;
    rt0.BlendOp = D3D12_BLEND_OP_ADD;
    rt0.SrcBlendAlpha = D3D12_BLEND_ZERO;
    rt0.DestBlendAlpha = D3D12_BLEND_ONE;
    rt0.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rt0.RenderTargetWriteMask = 0;

    desc.DepthStencilState.DepthEnable = false;
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    desc.DepthStencilState.StencilReadMask = 0xff;
    desc.DepthStencilState.StencilWriteMask = 0xff;

    desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
}

ComPtr<ID3D12PipelineState> DX12GraphicPipelineState::get(ComPtr<ID3D12Device> device)
{
    if (update) {
        device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
        update = false;
    }

    return pipelineState;
}

const DX12GraphicPipelineState::ID& DX12GraphicPipelineState::getID() const
{
    return id;
}

void DX12GraphicPipelineState::release()
{
    pipelineState.Reset();
}

bool DX12GraphicPipelineState::isValid() const
{
    return shaderProgramID != 0;
}

void DX12GraphicPipelineState::setRootSignature(ComPtr<ID3D12RootSignature> rootSignature)
{
    desc.pRootSignature = rootSignature.Get();
}

void DX12GraphicPipelineState::setInputLayout(DX12InputLayoutType type)
{
    if (inputLayoutType != type) {
        update = true;
        inputLayoutType = type;
        id.word0.info.inputLayout = (uint8_t)type;
        desc.InputLayout = DX12InputLayout::getLayout(type);
    }
}

void DX12GraphicPipelineState::setCullType(DX12CullType type)
{
    if (cullType != type) {
        update = true;
        cullType = type;
        id.word0.info.cull = (uint8_t)type;
        switch (type)
        {
        case DX12CullType::Cull_Off:
            desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            break;
        case DX12CullType::Cull_Back:
            desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
            break;
        case DX12CullType::Cull_Front:
            desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
            break;
        default:
            break;
        }
    }
}

void DX12GraphicPipelineState::setBlendFlags(Enum<DX12BlendFlags> flags)
{
    if (blendFlags != flags) {
        update = true;
        blendFlags = flags;
        id.word0.info.blend = flags;

        const uint8_t blendMaskValue = 0x7;

        uint8_t blendFlags = flags & blendMaskValue;

        auto& rt0 = desc.BlendState.RenderTarget[0];
        rt0.BlendEnable = blendFlags != 0;

        if (rt0.BlendEnable) {
            if (blendFlags == (uint8_t)DX12BlendFlags::BlendDefault) {
                rt0.SrcBlend = D3D12_BLEND_SRC_ALPHA;
                rt0.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
                rt0.SrcBlendAlpha = D3D12_BLEND_ONE;
                rt0.DestBlendAlpha = D3D12_BLEND_ZERO;
            }
            else if (blendFlags == (uint8_t)DX12BlendFlags::BlendAdd) {
                rt0.SrcBlend = D3D12_BLEND_ONE;
                rt0.DestBlend = D3D12_BLEND_ONE;
                rt0.SrcBlendAlpha = D3D12_BLEND_ZERO;
                rt0.DestBlendAlpha = D3D12_BLEND_ONE;
            }
            else if (blendFlags == (uint8_t)DX12BlendFlags::BlendPremultiplyAlpha) {
                rt0.SrcBlend = D3D12_BLEND_ONE;
                rt0.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
                rt0.SrcBlendAlpha = D3D12_BLEND_ZERO;
                rt0.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            }
            else if (blendFlags == (uint8_t)DX12BlendFlags::BlendMultiply) {
                rt0.SrcBlend = D3D12_BLEND_DEST_COLOR;
                rt0.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
                rt0.SrcBlendAlpha = D3D12_BLEND_ZERO;
                rt0.DestBlendAlpha = D3D12_BLEND_ONE;
            }
            else if (blendFlags == (uint8_t)DX12BlendFlags::BlendMask) {
                rt0.SrcBlend = D3D12_BLEND_ZERO;
                rt0.DestBlend = D3D12_BLEND_INV_SRC_COLOR;
                rt0.SrcBlendAlpha = D3D12_BLEND_ZERO;
                rt0.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            }
        }
        else {
            rt0.SrcBlend = D3D12_BLEND_ZERO;
            rt0.DestBlend = D3D12_BLEND_ONE;
            rt0.BlendOp = D3D12_BLEND_OP_ADD;
            rt0.SrcBlendAlpha = D3D12_BLEND_ZERO;
            rt0.DestBlendAlpha = D3D12_BLEND_ONE;
            rt0.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        }

        desc.BlendState.AlphaToCoverageEnable = flags.has(DX12BlendFlags::AlphaTest);
        rt0.RenderTargetWriteMask = flags.has(DX12BlendFlags::Write) ? D3D12_COLOR_WRITE_ENABLE_ALL : 0;
    }
}

void DX12GraphicPipelineState::setDepthFlags(Enum<DX12DepthFlags> flags)
{
    if (depthFlags != flags) {
        update = true;
        depthFlags = flags;
        id.word0.info.depth = flags;

        desc.DepthStencilState.DepthEnable = flags.has(DX12DepthFlags::Test);
        desc.DepthStencilState.DepthWriteMask = flags.has(DX12DepthFlags::Write) ?
            D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    }
}

void DX12GraphicPipelineState::setShader(const DX12ShaderProgramData& data)
{
    if (shaderProgramID != data.programID) {
        update = true;
        shaderProgramID = data.programID;
        id.word0.info.programID = shaderProgramID;

        if (data.VS != nullptr)
            desc.VS = CD3DX12_SHADER_BYTECODE(data.VS.Get());
        else
            desc.VS = {};

        if (data.GS != nullptr)
            desc.GS = CD3DX12_SHADER_BYTECODE(data.GS.Get());
        else
            desc.GS = {};

        if (data.HS != nullptr)
            desc.HS = CD3DX12_SHADER_BYTECODE(data.HS.Get());
        else
            desc.HS = {};

        if (data.DS != nullptr)
            desc.DS = CD3DX12_SHADER_BYTECODE(data.DS.Get());
        else
            desc.DS = {};

        if (data.PS != nullptr)
            desc.PS = CD3DX12_SHADER_BYTECODE(data.PS.Get());
        else
            desc.PS = {};
    }
}

void DX12GraphicPipelineState::setSampleCount(int count)
{
    if (desc.SampleDesc.Count != count) {
        update = true;
        desc.SampleDesc.Count = count;
        id.word0.info.sampleCount = count;
    }
}

void DX12GraphicPipelineState::setDSVFormat(DXGI_FORMAT dsvFormat)
{
    if (desc.DSVFormat != dsvFormat) {
        update = true;
        desc.DSVFormat = dsvFormat;
        id.word0.info.dsvFormat = dsvFormat;
    }
}

void DX12GraphicPipelineState::setRTVFormat(int count, DXGI_FORMAT* rtvFormat)
{
    if (desc.NumRenderTargets != count) {
        update = true;
        desc.NumRenderTargets = count;
    }
    for (int i = 0; i < 8; i++) {
        if (i < count) {
            if (desc.RTVFormats[i] != rtvFormat[i]) {
                update = true;
                desc.RTVFormats[i] = rtvFormat[i];
                id.word1.rtvFormats[i] = rtvFormat[i];
            }
        }
        else {
            desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
            id.word1.rtvFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
    }
}

DX12ComputePipelineState::DX12ComputePipelineState()
{
    reset();
}

DX12ComputePipelineState::~DX12ComputePipelineState()
{
    release();
}

void DX12ComputePipelineState::reset()
{
}

ComPtr<ID3D12PipelineState> DX12ComputePipelineState::get(ComPtr<ID3D12Device> device)
{
    if (update) {
        device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipelineState));
        update = false;
    }

    return pipelineState;
}

const DX12ComputePipelineState::ID& DX12ComputePipelineState::getID() const
{
    return id;
}

void DX12ComputePipelineState::release()
{
    pipelineState.Reset();
}

bool DX12ComputePipelineState::isValid() const
{
    return shaderProgramID != 0;
}

void DX12ComputePipelineState::setRootSignature(ComPtr<ID3D12RootSignature> rootSignature)
{
    desc.pRootSignature = rootSignature.Get();
}

void DX12ComputePipelineState::setShader(const DX12ShaderProgramData& data)
{
    if (shaderProgramID != data.programID) {
        update = true;
        desc.CS = CD3DX12_SHADER_BYTECODE(data.CS.Get());
        shaderProgramID = data.programID;
        id = shaderProgramID;
    }
}
