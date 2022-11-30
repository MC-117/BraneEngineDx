#include "TerrainRender.h"
#include "../RenderCore/RenderCore.h"

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

void TerrainRender::preRender()
{
}

void TerrainRender::render(RenderInfo& info)
{
    if (material == NULL || geometry == NULL || !geometry->meshPart.isValid() ||
        hidden || customRenaderSubmit)
        return;

    if (!customTransformSubmit)
        instanceID = info.sceneData->setMeshTransform(transformMat);

    if (!customTransformSubmit)
        info.sceneData->setMeshPartTransform(&geometry->meshPart, material, instanceID);
    MeshRenderCommand command;
    command.sceneData = info.sceneData;
    command.material = material;
    command.mesh = &geometry->meshPart;
    command.isStatic = isStatic;
    command.instanceID = instanceID;
    command.instanceIDCount = instanceCount;
    info.renderGraph->setRenderCommand(command);
}

Matrix4f TerrainRender::getTransformMatrix() const
{
    return transformMat;
}

IRendering::RenderType TerrainRender::getRenderType() const
{
    return IRendering::RenderType::Normal;
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

Shader* TerrainRender::getShader() const
{
    return material ? material->getShader() : NULL;
}
