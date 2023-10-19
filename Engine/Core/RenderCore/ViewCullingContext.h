#pragma once
#include "MeshBatchRenderData.h"
#include "CameraData.h"

struct ViewCullingKey
{
    struct Hasher
    {
        size_t operator()(const ViewCullingKey& k) const;
    };
    
    const MeshPart* meshPart = NULL;
    const MeshTransformDataArray* meshTransformDataArray = NULL;
    unsigned int instanceID = 0;

    ViewCullingKey(const MeshPart* meshPart, const MeshTransformDataArray* meshTransformDataArray, unsigned int instanceID);
    bool operator==(const ViewCullingKey& k) const;
};

enum SceneCullFlags : unsigned char
{
    SceneCull_NoneCulled = 0,
    SceneCull_FrustumCulled = 1 << 0,
    SceneCull_OcclusionCulled = 1 << 1,
};

struct ViewCullingContext : public IMeshBatchDrawCommandArray
{
public:
    GPUBuffer instanceCullingResultBuffer;
    GPUBuffer instanceDataBuffer;
    GPUBuffer commandBuffer;
    
    ViewCullingContext();
    void init();
    void setSourceBatchDrawCommandArray(IMeshBatchDrawCommandArray* array);
    void executeCulling(IRenderContext& context, IRenderData& cameraRenderData, MeshTransformRenderData& transformRenderData);
    virtual void bindInstanceBuffer(IRenderContext& context);
    virtual IGPUBuffer* getInstanceBuffer();
    virtual IGPUBuffer* getCommandBuffer();
    virtual unsigned int getInstanceCount() const;
    virtual unsigned int getCommandCount() const;

    virtual MeshBatchDrawCall* getMeshBatchDrawCall(const MeshBatchDrawKey& key);
    virtual MeshBatchDrawCall* setMeshBatchDrawCall(const MeshBatchDrawKey& key, unsigned int instanceIndex, unsigned int instanceCount = 1);
protected:
    static Material* instanceCullingMaterial;
    static ShaderProgram* instanceCullingProgram;
    static Material* pruneCommandMaterial;
    static ShaderProgram* pruneCommandProgram;
    static void loadDefaultResource();
    IMeshBatchDrawCommandArray* sourceBatchDrawCommandArray;
};

typedef TBatchDrawData<MeshTransformRenderData, ViewCullingContext> ViewCulledMeshBatchDrawData;
