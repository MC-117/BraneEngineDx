#pragma once

#include "../Core/Actor.h"
#include "PortalRender.h"

class ENGINE_API PortalActor : public Actor
{
public:
    Serialize(PortalActor, Actor);

    Ref<PortalActor> connectedPortal;

    PortalMeshRender portalMeshRender;
    PortalRender portalRender;
    PortalActor(const string& name = "PortalActor");

    void connect(PortalActor* portal);

    virtual Matrix4f getUnscaledTransformMat();

    virtual void begin();
    virtual void afterTick();
    virtual void prerender(SceneRenderData& sceneData);
    virtual void end();
    virtual Render* getRender();
    virtual unsigned int getRenders(vector<Render*>& renders);

    virtual void setHidden(bool value);
    virtual bool isHidden();

    static Serializable* instantiate(const SerializationInfo& from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
};
