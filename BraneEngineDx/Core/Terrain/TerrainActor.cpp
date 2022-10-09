#include "TerrainActor.h"
#include "../Physics/RigidBody.h"

SerializeInstance(TerrainActor);

TerrainActor::TerrainActor(const string& name) : Actor(name)
{
}

void TerrainActor::setGeometry(TerrainGeometry* geometry)
{
    terrainRender.setGeometry(geometry);
    updateCollision();
}

TerrainGeometry* TerrainActor::getGeometry() const
{
    return terrainRender.getGeometry();
}

void TerrainActor::updateCollision()
{
    if (terrainRender.geometry == NULL ||
        !terrainRender.geometry->isValid())
        return;
    if (rigidBody == NULL) {
        PhysicalMaterial physicalMaterial = PhysicalMaterial(0, TERRAIN);
        rigidBody = new RigidBody(*this, physicalMaterial);
        rigidBody->addCollider(terrainRender.geometry, COMPLEX);
    }
    else {
        terrainRender.geometry->updateCollisionShape();
    }
    PhysicalCollider* collider = rigidBody->getCollider();
    collider->setRotationOffset(Quaternionf::FromAngleAxis(PI * 0.5f, Vector3f::UnitZ()) *
        Quaternionf::FromAngleAxis(PI * 0.5f, Vector3f::UnitX()));
}

void TerrainActor::prerender(SceneRenderData& sceneData)
{
    terrainRender.transformMat = transformMat;
}

Render* TerrainActor::getRender()
{
    return &terrainRender;
}

unsigned int TerrainActor::getRenders(vector<Render*>& renders)
{
    renders.push_back(&terrainRender);
    return 1;
}

void TerrainActor::setHidden(bool value)
{
    terrainRender.hidden = value;
}

bool TerrainActor::isHidden()
{
    return terrainRender.hidden;
}

Serializable* TerrainActor::instantiate(const SerializationInfo& from)
{
    return nullptr;
}

bool TerrainActor::deserialize(const SerializationInfo& from)
{
    return false;
}

bool TerrainActor::serialize(SerializationInfo& to)
{
    return false;
}
