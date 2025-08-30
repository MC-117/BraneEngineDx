#pragma once

#include "Config.h"
#include "GraphicType.h"

class RenderTarget;
class ShaderProgram;

enum PipelineStateType
{
    PSO_Graphics,
    PSO_Compute
};

class ENGINE_API IPipelineState
{
public:
    virtual ~IPipelineState() = default;
    
    virtual PipelineStateType getType() const = 0;
    virtual bool isValid() const = 0;
    virtual bool init() = 0;
};

struct ENGINE_API GraphicsPipelineStateId
{
    static constexpr int NumBit64 = 11;
    uint64_t packedValue[NumBit64];

    bool operator==(const GraphicsPipelineStateId& id) const;

    bool operator!=(const GraphicsPipelineStateId& id) const;

    bool operator<(const GraphicsPipelineStateId& id) const;
};

template <>
struct ENGINE_API std::hash<GraphicsPipelineStateId>
{
    std::size_t operator()(const GraphicsPipelineStateId& id) const noexcept;
};

struct ENGINE_API GraphicsPipelineStateDesc
{
    union
    {
        struct
        {
            ShaderProgram* program;
            RenderMode renderMode;
            TexInternalType rtvFormats[MaxRenderTargets];
            uint32_t flags;
            TexInternalType dsvFormat : 8;
            MeshType meshType : 4;
            CullType cullType : 4;
            uint8_t renderTargetCount : 4;
            uint8_t sampleCount : 4;
            uint8_t pad;
        };

        GraphicsPipelineStateId id;
    };

    GraphicsPipelineStateDesc();
    GraphicsPipelineStateDesc(const GraphicsPipelineStateDesc& desc);
    GraphicsPipelineStateDesc(GraphicsPipelineStateDesc&& desc);

    static GraphicsPipelineStateDesc forScreen(ShaderProgram* shaderProgram, const RenderTarget* renderTarget, BlendMode blendMode);
    static GraphicsPipelineStateDesc forLines(ShaderProgram* shaderProgram, const RenderTarget* renderTarget, DepthStencilMode depthStencilMode);
};

static_assert(sizeof(GraphicsPipelineStateDesc) == sizeof(GraphicsPipelineStateId));

class ENGINE_API GraphicsPipelineState : public IPipelineState
{
public:
    GraphicsPipelineStateDesc desc;

    GraphicsPipelineState(const GraphicsPipelineStateDesc& desc) : desc(desc) { }
    virtual ~GraphicsPipelineState() = default;

    virtual PipelineStateType getType() const { return PSO_Graphics; }

    virtual bool isValid() const = 0;

    virtual bool init() = 0;
};

typedef size_t ComputePipelineStateId;

struct ENGINE_API ComputePipelineStateDesc
{
    union
    {
        ShaderProgram* program;
        ComputePipelineStateId id;
    };

    ComputePipelineStateDesc(ShaderProgram* shaderProgram);
    ComputePipelineStateDesc(const ComputePipelineStateDesc& desc);
    ComputePipelineStateDesc(ComputePipelineStateDesc&& desc);
};

class ENGINE_API ComputePipelineState : public IPipelineState
{
public:
    ComputePipelineStateDesc desc;

    ComputePipelineState(const ComputePipelineStateDesc& desc) : desc(desc) { }
    virtual ~ComputePipelineState() = default;

    virtual PipelineStateType getType() const { return PSO_Compute; }

    virtual bool isValid() const = 0;

    virtual bool init() = 0;
};
