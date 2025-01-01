#pragma once

#include "PortalActor.h"
#include "../Core/RenderTarget.h"

class World;
class ICameraManager;
class PortalCameraRenderData;

class ENGINE_API PortalManager : public ObjectBehavior
{
public:
    Serialize(PortalManager, ObjectBehavior);
    
    virtual bool init(Object* object);

    virtual void begin();
    virtual void tick(float deltaTime);
    virtual void afterTick();
    virtual void prerender(SceneRenderData& sceneData);
    virtual void end();

    void registerPortalActor(PortalActor& portal);
    void unregisterPortalActor(PortalActor& portal);

    const CameraRenderData* getPortalRenderData(int portalID) const;

    static Serializable* instantiate(const SerializationInfo& from);
protected:
    World* world = NULL;
    RenderTarget portalRenderTarget;
    set<Ref<PortalActor>> portalActors;
    int maxPortalDepth = 5;

    vector<CameraRenderData*> portalRenderDatas;
    int visiablePortalCount = 0;
    PortalCameraRenderData* fetchPortalRenderData(CameraRenderData* sourceRenderData);

    void solvePortalRendering(ICameraManager& cameraManager);

    void registerExistedPortalActor();
};
