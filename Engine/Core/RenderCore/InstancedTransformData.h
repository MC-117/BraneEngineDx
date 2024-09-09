#pragma once

#include "TransformRenderData.h"

struct InstanceGernerationConfig
{
    enum Type
    {
        DensityBox, DensitySphere
    } type = DensityBox;

    Matrix4f rootTransform;
    union
    {
        struct
        {
            Vector2f bound;
            float density = 0;
        } densityBox;

        struct
        {
            float radius = 0;
            float density = 0;
        } densitySphere;
    } data;

    unsigned int getInstanceCount() const;
};

struct InstancedMeshTransformRenderData : public MeshTransformRenderData
{
    MeshTransformDataArray::ReservedData generateInstances(const InstanceGernerationConfig& config);
protected:
    void generateInstanceBox(MeshTransformData* data, unsigned int count, const InstanceGernerationConfig& config);
    void generateInstanceSphere(MeshTransformData* data, unsigned int count, const InstanceGernerationConfig& config);
};
