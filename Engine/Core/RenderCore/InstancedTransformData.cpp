#include "InstancedTransformData.h"

unsigned int InstanceGernerationConfig::getInstanceCount() const
{
    switch (type) {
    case DensityBox: return unsigned int(data.densityBox.bound.y() / data.densityBox.bound.x() * data.densityBox.density + 1) * unsigned int(data.densityBox.density + 1);
    case DensitySphere: return 0;
    default: return 0;
    }
}

MeshTransformDataArray::ReservedData InstancedMeshTransformRenderData::generateInstances(const InstanceGernerationConfig& config)
{
    unsigned int count = config.getInstanceCount();
    MeshTransformDataArray::ReservedData reservedData = addMeshTransform(count);
    MeshTransformData* data = reservedData.beginModify();
    switch (config.type) {
    case InstanceGernerationConfig::DensityBox: generateInstanceBox(data, count, config); break;
    case InstanceGernerationConfig::DensitySphere: generateInstanceSphere(data, count, config); break;
    }
    return MeshTransformDataArray::ReservedData::none;
}

void InstancedMeshTransformRenderData::generateInstanceBox(MeshTransformData* data, unsigned int count, const InstanceGernerationConfig& config)
{
    
}

void InstancedMeshTransformRenderData::generateInstanceSphere(MeshTransformData* data, unsigned count, const InstanceGernerationConfig& config)
{
}
