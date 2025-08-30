#pragma once
#include "TransformRenderData.h"

typedef TMeshTransformDataArray<Matrix4f, MatrixUploadOp> SkeletonTransformArray;

struct SkeletonRenderData : public IRenderData
{
    static Material* uploadTransformMaterial;
    static ShaderProgram* uploadTransformProgram;
    static ComputePipelineState* uploadTransformPSO;

    SkeletonTransformArray skeletonTransformArray;

    GPUBuffer transformUploadBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(SkeletonTransformArray::DataType));
    GPUBuffer transformUploadIndexBuffer = GPUBuffer(GB_Storage, GBF_UInt);
    GPUBuffer transformBuffer = GPUBuffer(GB_Storage, GBF_Struct, sizeof(SkeletonTransformArray::DataType), GAF_ReadWrite, CAF_None);

    void setBoneCount(unsigned int count);

    void updateBoneTransform(const SkeletonTransformArray::DataType& data, unsigned int index);

    static void loadDefaultResource();

    virtual void create();
    virtual void release();
    virtual void upload();
    virtual void bind(IRenderContext& context);
    void clean();
};
