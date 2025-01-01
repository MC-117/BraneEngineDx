#pragma once

#include "../Core/MeshRender.h"
#include "../Core/CameraRender.h"

class PortalMeshRender : public MeshRender
{
public:
    PortalMeshRender();
    virtual ~PortalMeshRender();

    void setInternalMesh();

    void setPortalID(int portalID);
    int getPortalID() const;

    virtual void getMeshTransformData(MeshTransformData* data) const;

    virtual void render(RenderInfo& info);
protected:
    int portalID = -1;
    static bool isInited;
    static Mesh planeMesh;
    static Material* portalMaterial;

    Material* portalMaterialInstance = NULL;
    void initialPlaneMesh();
};

class PortalRender : public CameraRender
{
public:
    Camera* mainCamera = NULL;
    
    PortalRender();

    int getCameraRenderID() const;

    void setRenderData(CameraRenderData* renderData);
    CameraRenderData* getRenderData();
    virtual void render(RenderInfo& info);
};
