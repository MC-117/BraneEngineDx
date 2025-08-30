#include "DecalActor.h"
#include "RenderCore/RenderThread.h"

SerializeInstance(DecalActor);

DecalActor::DecalActor(const string& name) : Actor(name)
{
}

void DecalActor::setMesh(Mesh* mesh)
{
    decalRender.setMesh(mesh);
}

void DecalActor::setDefaultMesh()
{
    decalRender.setDecalBoxMesh();
}

void DecalActor::prerender(SceneRenderData& sceneData)
{
    if (decalRender.getMesh() == NULL) {
        setDefaultMesh();
    }
    Actor::prerender(sceneData);
    decalRender.transformMat = transformMat;
    MeshTransformData data;
    getMeshTransformData(&data);
    decalRender.getMeshTransformData(&data);
    RENDER_THREAD_ENQUEUE_TASK(DecalActorUpdateTransform, ([this, data] (RenderThreadContext& context)
    {
        unsigned int renderInstanceID = context.sceneRenderData->setMeshTransform(data);
        decalRender.setInstanceInfo(renderInstanceID, 1);
    }));
}

Render* DecalActor::getRender()
{
    return &decalRender;
}

unsigned int DecalActor::getRenders(vector<Render*>& renders)
{
    renders.push_back(&decalRender);
    return 1;
}

void DecalActor::setHidden(bool value)
{
    decalRender.hidden = value;
}

bool DecalActor::isHidden()
{
    return decalRender.hidden;
}

Serializable* DecalActor::instantiate(const SerializationInfo& from)
{
    DecalActor* actor = new DecalActor(from.name);
    ChildrenInstantiate(Object, from, actor);
    return actor;
}

bool DecalActor::deserialize(const SerializationInfo& from)
{
    if (!::Actor::deserialize(from))
        return false;
    return decalRender.deserialize(from);
}

bool DecalActor::serialize(SerializationInfo& to)
{
    if (!Actor::serialize(to))
        return false;
    return decalRender.serialize(to);
}
