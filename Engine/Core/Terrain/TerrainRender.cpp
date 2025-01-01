#include "TerrainRender.h"
#include "../RenderCore/RenderCore.h"
#include "../RenderCore/RenderCoreUtility.h"
#include "../RenderCore/TerrainRenderData.h"
#include "../Camera.h"

TerrainRender::TerrainRender() : Render()
{
    canCastShadow = true;
}

void TerrainRender::setGeometry(TerrainGeometry* geometry)
{
    this->geometry = geometry;
}

TerrainGeometry* TerrainRender::getGeometry() const
{
    return geometry;
}

void TerrainRender::setInstanceInfo(unsigned int instanceID, unsigned int instanceCount)
{
    this->instanceID = instanceID;
    this->instanceCount = instanceCount;
}

void TerrainRender::getMeshTransformData(MeshTransformData* data) const
{
    if (geometry == NULL || !geometry->isValid())
        return;
    data->localCenter = geometry->getCenter();
    data->localExtent = geometry->getExtent();
    data->localRadius = data->localExtent.norm();
}

void TerrainRender::preRender(PreRenderInfo& info)
{
}

void TerrainRender::render(RenderInfo& info)
{
    if (material == NULL || geometry == NULL ||
        !geometry->meshPart.isValid() || hidden)
        return;

    RENDER_THREAD_ENQUEUE_TASK(TerrainRenderUpdate, ([this, meshPart = &geometry->meshPart,
        materialRenderData = material->getMaterialRenderData(), instanceID = instanceID, instanceCount = instanceCount] (RenderThreadContext& context)
    {
        MeshRenderCommand command;
        command.sceneData = context.sceneRenderData;
        command.materialRenderData = materialRenderData;
        command.mesh = meshPart;
        command.instanceID = instanceID;
        command.instanceIDCount = instanceCount;
        MeshBatchDrawData batchDrawData = context.sceneRenderData->getBatchDrawData(isStatic);
        command.batchDrawData = batchDrawData;
        MeshBatchDrawKey renderKey(command.mesh, command.materialRenderData, batchDrawData.transformData->getMeshTransform(instanceID).isNegativeScale());
        command.meshBatchDrawCall = batchDrawData.batchDrawCommandArray->setMeshBatchDrawCall(renderKey, instanceID, instanceCount);
        command.reverseCullMode = renderKey.negativeScale;
        collectRenderDataInCommand(context.renderGraph, command);
        context.renderGraph->setRenderCommand(command);
    }));
}

Matrix4f TerrainRender::getTransformMatrix() const
{
    return transformMat;
}

IRendering::RenderType TerrainRender::getRenderType() const
{
    return IRendering::RenderType::Normal_Render;
}

Shape* TerrainRender::getShape() const
{
    return geometry;
}

Material* TerrainRender::getMaterial(unsigned int index)
{
    return index != 0 ? NULL : material;
}

bool TerrainRender::getMaterialEnable(unsigned int index)
{
    return index != 0 ? false : material;
}

unsigned int TerrainRender::getInstanceID() const
{
    return instanceID;
}

unsigned int TerrainRender::getInstanceCount() const
{
    return instanceCount;
}

Shader* TerrainRender::getShader() const
{
    return material ? material->getShader() : NULL;
}

TerrainRenderData* TerrainRender::getRenderData()
{
    if (terrainRenderData == NULL) {
        terrainRenderData = new TerrainRenderData();
    }
    // terrainRenderData->quadTree.setTerrainData(data);
    return terrainRenderData;
}
