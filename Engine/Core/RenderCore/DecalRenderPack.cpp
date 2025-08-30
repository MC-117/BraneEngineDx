#include "DecalRenderPack.h"
#include "RenderCoreUtility.h"
#include "RenderTask.h"
#include "../Profile/RenderProfile.h"

CullType DecalRenderCommand::getCullType() const
{
    return reverseCullMode ? Cull_Front : Cull_Back;
}

RenderMode DecalRenderCommand::getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const
{
    if (passName != "Geometry"_N) {
        throw std::invalid_argument("Pass name must be \"Geometry\"");
    }

    RenderMode renderMode = MeshRenderCommand::getRenderMode(passName, cameraRenderData);

    renderMode.dsMode.depthTest = true;
    renderMode.dsMode.depthWrite = false;
    renderMode.dsMode.stencilTest = true;
    renderMode.dsMode.stencilReadMask = 0x1;
    renderMode.dsMode.stencilWriteMask = 0x1;
    renderMode.dsMode.stencilComparion_back = RCT_Equal;
    renderMode.dsMode.stencilComparion_front = RCT_Equal;
    renderMode.dsMode.stencilFailedOp_back = SOT_Keep;
    renderMode.dsMode.stencilFailedOp_front = SOT_Keep;
    renderMode.dsMode.stencilDepthFailedOp_back = SOT_Keep;
    renderMode.dsMode.stencilDepthFailedOp_front = SOT_Keep;
    renderMode.dsMode.stencilPassOp_back = SOT_Zero;
    renderMode.dsMode.stencilPassOp_front = SOT_Zero;
    renderMode.dsMode.accessFlag = DSA_DepthReadOnly;

    auto setRenderMode = [&](RenderTargetMode& mode, DecalMask mask) {
        mode.setBlendMode(BM_Default);
        mode.enableBlend = decalMask.has(mask);
        mode.writeMask = mode.enableBlend ? CWM_RGB : CWM_None;
    };

    setRenderMode(renderMode[0], Decal_BaseColor);
    setRenderMode(renderMode[1], Decal_Normal);
    setRenderMode(renderMode[4], Decal_Emission);

    uint8_t rmsaMask = CWM_None;

    if (decalMask.has(Decal_Roughness)) {
        rmsaMask |= CWM_R;
    }

    if (decalMask.has(Decal_Metallic)) {
        rmsaMask |= CWM_G;
    }

    if (decalMask.has(Decal_Specular)) {
        rmsaMask |= CWM_B;
    }

    renderMode[2].setBlendMode(BM_Default);
    renderMode[2].enableBlend = rmsaMask != 0;
    renderMode[2].writeMask = (ColorWriteMask)rmsaMask;

    renderMode[3].enableBlend = false;
    renderMode[3].writeMask = CWM_None;
    
    return renderMode;
}

IRenderPack* DecalRenderCommand::createRenderPack(SceneRenderData& sceneData, RenderCommandList& commandList) const
{
    return new DecalRenderPack();
}

DecalRenderPack::DecalRenderPack() : materialData(NULL)
{
}

bool DecalRenderPack::setRenderCommand(const IRenderCommand& command, const RenderTask& task)
{
    const DecalRenderCommand* decalRenderCommand = dynamic_cast<const DecalRenderCommand*>(&command);
    if (decalRenderCommand == NULL)
        return false;

    materialData = command.materialRenderData;
    if (materialData == NULL)
        return false;

    localToDecalClip = decalRenderCommand->localToDecalClip;

    if (decalRenderCommand->meshBatchDrawCall
        && decalRenderCommand->meshBatchDrawCall->getInstanceCount() > 0
        && decalRenderCommand->meshBatchDrawCall->getDrawCommandCount() > 0) {
        auto& pso = meshBatchDrawCalls.try_emplace(decalRenderCommand->meshBatchDrawCall, nullptr).first->second;
        if (pso == NULL) {
            IMaterial* depthVariant = decalRenderCommand->materialRenderData->getVariant(Shader_Depth);
            auto& rawDesc = task.graphicsPipelineState->desc;
            GraphicsPipelineStateDesc stencilOnlyDesc;
            stencilOnlyDesc.cullType = Cull_Off;
            stencilOnlyDesc.dsvFormat = rawDesc.dsvFormat;
            stencilOnlyDesc.meshType = rawDesc.meshType;
            stencilOnlyDesc.program = depthVariant->program;
            stencilOnlyDesc.renderMode = RenderMode(BM_DepthOnly);
            stencilOnlyDesc.renderMode.dsMode.depthWrite = false;
            stencilOnlyDesc.renderMode.dsMode.depthComparion = RCT_LessEqual;
            stencilOnlyDesc.renderMode.dsMode.stencilTest = true;
            stencilOnlyDesc.renderMode.dsMode.stencilReadMask = 0x1;
            stencilOnlyDesc.renderMode.dsMode.stencilWriteMask = 0x1;
            stencilOnlyDesc.renderMode.dsMode.stencilComparion_back = RCT_Always;
            stencilOnlyDesc.renderMode.dsMode.stencilComparion_front = RCT_Always;
            stencilOnlyDesc.renderMode.dsMode.stencilFailedOp_back = SOT_Keep;
            stencilOnlyDesc.renderMode.dsMode.stencilFailedOp_front = SOT_Keep;
            stencilOnlyDesc.renderMode.dsMode.stencilDepthFailedOp_back = SOT_Keep;
            stencilOnlyDesc.renderMode.dsMode.stencilDepthFailedOp_front = SOT_Keep;
            stencilOnlyDesc.renderMode.dsMode.stencilPassOp_back = SOT_Invert;
            stencilOnlyDesc.renderMode.dsMode.stencilPassOp_front = SOT_Invert;
            pso = fetchPSOIfDescChangedThenInit(NULL, stencilOnlyDesc);
        }
    }

    return true;
}

void DecalRenderPack::excute(IRenderContext& context, RenderTask& task, RenderTaskContext& taskContext)
{
    if (meshBatchDrawCalls.empty())
        return;

    if (taskContext.materialVariant != task.materialVariant) {
        taskContext.materialVariant = task.materialVariant;

        static const ShaderPropertyName localToDecalClipName = "localToDecalClip";
        task.materialVariant->desc.matrixField[localToDecalClipName] = localToDecalClip;
        bindMaterial(context, task.materialVariant);
    }

    static const ShaderPropertyName sceneDepthMapName = "sceneDepthMap";
    
    for (auto& item : meshBatchDrawCalls) {
        materialData->desc.currentPass = 0;

        context.setDrawInfo(0, materialData->desc.passNum, materialData->desc.materialID); 
        context.bindDrawInfo();
        
        if (item.second) {
            context.setStencilRef(0);
            context.bindPipelineState(item.second);
            
            unsigned int commandOffset = item.first->getDrawCommandOffset();
            unsigned int commandEnd = commandOffset + item.first->getDrawCommandCount();
            for (; commandOffset < commandEnd; commandOffset++) {
                RENDER_DESC_SCOPE(context, DrawMesh, "DecalStencil Material(%s)", AssetInfo::getPath(materialData->material).c_str());
                context.drawMeshIndirect(taskContext.batchDrawData.batchDrawCommandArray->getCommandBuffer(), sizeof(DrawElementsIndirectCommand) * commandOffset);
            }

            context.bindPipelineState(task.graphicsPipelineState);
        }
            
        context.bindTexture(taskContext.renderTarget->getDepthTexture()->getVendorTexture(), sceneDepthMapName);

        context.setStencilRef(1);
        unsigned int commandOffset = item.first->getDrawCommandOffset();
        unsigned int commandEnd = commandOffset + item.first->getDrawCommandCount();
        for (; commandOffset < commandEnd; commandOffset++) {
            RENDER_DESC_SCOPE(context, DrawMesh, "Decal Material(%s)", AssetInfo::getPath(materialData->material).c_str());
            context.drawMeshIndirect(taskContext.batchDrawData.batchDrawCommandArray->getCommandBuffer(), sizeof(DrawElementsIndirectCommand) * commandOffset);
        }

        context.unbindBufferBase(sceneDepthMapName);
    }
    
    context.setStencilRef(0);
}

void DecalRenderPack::reset()
{
    materialData = NULL;
    meshBatchDrawCalls.clear();
}
