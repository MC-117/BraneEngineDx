#include "PortalActor.h"

#include "PortalManager.h"
#include "../Core/RenderCore/RenderThread.h"
#include "../Core/ObjectUltility.h"
#include "../Core/Utility/MathUtility.h"

SerializeInstance(PortalActor);

PortalActor::PortalActor(const string& name) : Actor(name)
{
    portalMeshRender.setInternalMesh();
}

void PortalActor::connect(PortalActor* portal)
{
    if (connectedPortal == portal)
        return;
    if (connectedPortal)
        connectedPortal->connectedPortal = NULL;
    connectedPortal = portal;
    if (portal)
        portal->connect(this);
}

Matrix4f PortalActor::getUnscaledTransformMat()
{
    Matrix4f mat = getMatrix(WORLD);
    Vector3f worldPos, worldScale;
    Quaternionf worldRot;
    mat.decompose(worldPos, worldRot, worldScale);
    return Math::getTransformMatrix(worldPos, worldRot, Vector3f::Ones());
}

void PortalActor::begin()
{
    Actor::begin();
    World* world = getRootWorld(*this);
    assert(world);
    
    if (PortalManager* manager = getObjectBehavior<PortalManager>(*world)) {
        manager->registerPortalActor(*this);
    }

}

void PortalActor::afterTick()
{
    Actor::afterTick();
}

void PortalActor::prerender(SceneRenderData& sceneData)
{
    Actor::prerender(sceneData);
    
    portalMeshRender.transformMat = transformMat;
    MeshTransformData data;
    getMeshTransformData(&data);
    portalMeshRender.getMeshTransformData(&data);
    RENDER_THREAD_ENQUEUE_TASK(MeshActorUpdateTransform, ([this, data] (RenderThreadContext& context)
    {
        unsigned int renderInstanceID = context.sceneRenderData->setMeshTransform(data);
        portalMeshRender.setInstanceInfo(renderInstanceID, 1);
    }));

    portalRender.transformMat = transformMat;
}

void PortalActor::end()
{
    Actor::end();
    
    World* world = getRootWorld(*this);
    assert(world);

    PortalManager* manager = getObjectBehavior<PortalManager>(*world);
    assert(manager);

    manager->registerPortalActor(*this);
}

Render* PortalActor::getRender()
{
    return &portalMeshRender;
}

unsigned int PortalActor::getRenders(vector<Render*>& renders)
{
    renders.emplace_back(&portalMeshRender);
    renders.emplace_back(&portalRender);
    return 2;
}

void PortalActor::setHidden(bool value)
{
    Actor::setHidden(value);
    portalMeshRender.hidden = value;
    portalRender.hidden = value;
}

bool PortalActor::isHidden()
{
    return portalMeshRender.hidden;
}

Serializable* PortalActor::instantiate(const SerializationInfo& from)
{
    return new PortalActor(from.name);
}

bool PortalActor::deserialize(const SerializationInfo& from)
{
    return Actor::deserialize(from);
}

bool PortalActor::serialize(SerializationInfo& to)
{
    return Actor::serialize(to);
}
