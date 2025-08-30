#pragma once

#include "Actor.h"
#include "DecalRender.h"

class DecalActor : public Actor
{
public:
    Serialize(DecalActor, Actor);

    DecalRender decalRender;

    DecalActor(const string& name = "DecalActor");

    virtual void setMesh(Mesh* mesh);
    virtual void setDefaultMesh();

    virtual void prerender(SceneRenderData& sceneData);
    virtual Render* getRender();
    virtual unsigned int getRenders(vector<Render*>& renders);

    virtual void setHidden(bool value);
    virtual bool isHidden();

    static Serializable* instantiate(const SerializationInfo& from);
    virtual bool deserialize(const SerializationInfo& from);
    virtual bool serialize(SerializationInfo& to);
};
