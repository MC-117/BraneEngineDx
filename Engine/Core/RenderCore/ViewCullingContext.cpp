#include "ViewCullingContext.h"

#include "RenderCoreUtility.h"
#include "SceneRenderData.h"
#include "../Asset.h"
#include "../Mesh.h"
#include "../Utility/hash.h"
#include "../Utility/Parallel.h"
#include "../Utility/RenderUtility.h"
#include "Core/Profile/RenderProfile.h"

size_t ViewCullingKey::Hasher::operator()(const ViewCullingKey& k) const
{
    size_t hash = (size_t)k.meshPart;
    hash_combine(hash, (size_t)k.meshTransformDataArray);
    hash_combine(hash, (size_t)k.instanceID);
    return hash;
}

ViewCullingKey::ViewCullingKey(const MeshPart* meshPart
    , const MeshTransformDataArray* meshTransformDataArray
    , unsigned int instanceID)
    : meshPart(meshPart)
    , meshTransformDataArray(meshTransformDataArray)
    , instanceID(instanceID)
{
}

bool ViewCullingKey::operator==(const ViewCullingKey& k) const
{
    return meshPart == k.meshPart
    && meshTransformDataArray == k.meshTransformDataArray
    && instanceID == k.instanceID;
}

Material* ViewCullingContext::instanceCullingMaterial = NULL;
ShaderProgram* ViewCullingContext::instanceCullingProgram = NULL;
ComputePipelineState* ViewCullingContext::instanceCullingPSO = NULL;
Material* ViewCullingContext::pruneCommandMaterial = NULL;
ShaderProgram* ViewCullingContext::pruneCommandProgram = NULL;
ComputePipelineState* ViewCullingContext::pruneCommandPSO = NULL;

ViewCullingContext::ViewCullingContext()
    : instanceCullingResultBuffer(GB_Storage, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
    , instanceDataBuffer(GB_Vertex, GBF_UInt2, 0, GAF_ReadWrite, CAF_None)
    , commandBuffer(GB_Command, GBF_UInt, 0, GAF_ReadWrite, CAF_None)
{
}

void ViewCullingContext::init()
{
    loadDefaultResource();
}

bool ViewCullingContext::applyValidViewCullingContext(BatchDrawData& batchDrawData)
{
    if (sourceBatchDrawCommandArray && sourceBatchDrawCommandArray == batchDrawData.batchDrawCommandArray) {
        batchDrawData.batchDrawCommandArray = this;
        return true;
    }
    return false;
}

void ViewCullingContext::setSourceBatchDrawCommandArray(IMeshBatchDrawCommandArray* array)
{
    sourceBatchDrawCommandArray = array;
}

void ViewCullingContext::executeCulling(IRenderContext& context, IRenderData& cameraRenderData, MeshTransformRenderData& transformRenderData)
{
    if (sourceBatchDrawCommandArray == NULL)
        return;
    static const ShaderPropertyName instanceCullingResultName("instanceCullingResult");
    static const ShaderPropertyName instanceDataName("instanceData");
    static const ShaderPropertyName commandsName("commands");
    static const ShaderPropertyName prunedInstanceDataName("prunedInstanceData");
    static const ShaderPropertyName prunedCommandsName("prunedCommands");

    const unsigned int transformCount = transformRenderData.getTransformCount();
    const unsigned int instanceCount = sourceBatchDrawCommandArray->getInstanceCount();
    const unsigned int commandCount = sourceBatchDrawCommandArray->getCommandCount();

    if (instanceCount == 0 || commandCount == 0)
        return;

    RENDER_SCOPE(context, ViewCulling);
    
    instanceCullingResultBuffer.resize(transformCount);
    instanceDataBuffer.resize(instanceCount);
    commandBuffer.resize(commandCount * sizeof(DrawElementsIndirectCommand) / sizeof(unsigned int));

    context.bindPipelineState(instanceCullingPSO);
    cameraRenderData.bind(context);
    transformRenderData.bind(context);
    context.bindBufferBase(instanceCullingResultBuffer.getVendorGPUBuffer(), instanceCullingResultName, { true });
    context.dispatchCompute(ceilf(instanceCount / (float)instanceCullingMaterial->getLocalSize().x()), 1, 1);

    context.unbindBufferBase(instanceCullingResultName);

    context.bindPipelineState(pruneCommandPSO);
    context.bindBufferBase(instanceCullingResultBuffer.getVendorGPUBuffer(), instanceCullingResultName);
    context.bindBufferBase(sourceBatchDrawCommandArray->getInstanceBuffer(), instanceDataName);
    context.bindBufferBase(sourceBatchDrawCommandArray->getCommandBuffer(), commandsName);
    context.bindBufferBase(instanceDataBuffer.getVendorGPUBuffer(), prunedInstanceDataName, { true });
    context.bindBufferBase(commandBuffer.getVendorGPUBuffer(), prunedCommandsName, { true });
    context.clearOutputBufferUint(commandBuffer.getVendorGPUBuffer(), Vector4u::Zero());
    context.dispatchCompute(ceilf(commandCount / (float)pruneCommandMaterial->getLocalSize().x()), 1, 1);

    context.unbindBufferBase(prunedInstanceDataName);
    context.unbindBufferBase(prunedCommandsName);
}

void ViewCullingContext::bindInstanceBuffer(IRenderContext& context)
{
    context.bindBufferBase(instanceDataBuffer.getVendorGPUBuffer(), TRANS_INDEX_BIND_INDEX);
}

IGPUBuffer* ViewCullingContext::getInstanceBuffer()
{
    return instanceDataBuffer.getVendorGPUBuffer();
}

IGPUBuffer* ViewCullingContext::getCommandBuffer()
{
    return commandBuffer.getVendorGPUBuffer();
}

unsigned ViewCullingContext::getInstanceCount() const
{
    return instanceDataBuffer.size();
}

unsigned ViewCullingContext::getCommandCount() const
{ 
    return commandBuffer.size() / (sizeof(DrawElementsIndirectCommand) / sizeof(unsigned int));
}

MeshBatchDrawCall* ViewCullingContext::getMeshBatchDrawCall(const MeshBatchDrawKey& key)
{
    if (sourceBatchDrawCommandArray == NULL)
        return NULL;
    return sourceBatchDrawCommandArray->getMeshBatchDrawCall(key);
}

MeshBatchDrawCall* ViewCullingContext::setMeshBatchDrawCall(const MeshBatchDrawKey& key, unsigned int instanceIndex, unsigned int instanceCount)
{
    if (sourceBatchDrawCommandArray == NULL)
        return NULL;
    return sourceBatchDrawCommandArray->setMeshBatchDrawCall(key, instanceIndex, instanceCount);
}

void ViewCullingContext::loadDefaultResource()
{
    if (instanceCullingProgram == NULL) {
        instanceCullingMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/InstanceCulling.mat");
        if (instanceCullingMaterial == NULL)
            return;
        instanceCullingProgram = instanceCullingMaterial->getShader()->getProgram(Shader_Default);
    }
    if (instanceCullingProgram == NULL)
        throw runtime_error("instanceCulling shader program is invalid");
    instanceCullingPSO = fetchPSOIfDescChangedThenInit(instanceCullingPSO, instanceCullingProgram);

    if (pruneCommandProgram == NULL) {
        pruneCommandMaterial = getAssetByPath<Material>("Engine/Shaders/Pipeline/PruneCommandWithCullingData.mat");
        if (pruneCommandMaterial == NULL)
            return;
        pruneCommandProgram = pruneCommandMaterial->getShader()->getProgram(Shader_Default);
    }
    if (pruneCommandProgram == NULL)
        throw runtime_error("PruneCommandWithCullingData shader program is invalid");
    pruneCommandPSO = fetchPSOIfDescChangedThenInit(pruneCommandPSO, pruneCommandProgram);
}
