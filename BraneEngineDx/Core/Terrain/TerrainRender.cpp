#include "TerrainRender.h"

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
        instanceID = info.cmdList->setMeshTransform(transformMat);

    if (!customTransformSubmit)
        info.cmdList->setMeshPartTransform(&geometry->meshPart, material, instanceID);
    info.cmdList->setRenderCommand({ material, info.camera, &geometry->meshPart }, isStatic);
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

int TerrainRender::getRenderResource(vector<RenderResource>& resources)
{
    if (geometry && geometry->isValid() && material) {
        RenderResource& resource = resources.emplace_back();
        resource.enable = true;
        resource.instanceID = instanceID;
        resource.instanceIDCount = instanceCount;
        resource.material = material;
        resource.meshPart = &geometry->meshPart;
        return 1;
    }
    return 0;
}
