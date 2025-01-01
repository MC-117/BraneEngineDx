#include "PortalRender.h"
#include "../Core/RenderCore/RenderThread.h"
#include "../Core/RenderCore/RenderCoreUtility.h"
#include "../Core/RenderCore/RenderCore.h"
#include "../Core/Camera.h"
#include "../Core/Asset.h"

struct PortalMeshRenderCommand : public MeshRenderCommand
{
    virtual RenderMode getRenderMode(const Name& passName, const CameraRenderData* cameraRenderData) const
    {
        const int renderOrder = 2499;
        RenderMode renderMode = RenderMode(renderOrder, BM_Default);
        renderMode.mode.depthWrite = false;
        renderMode.mode.stencilTest = materialRenderData->desc.enableStencilTest;
        renderMode.mode.stencilPassOp_front = SOT_Replace;
        renderMode.mode.stencilPassOp_back = SOT_Replace;
        renderMode.mode.stencilComparion_front = RCT_Less;
        renderMode.mode.stencilComparion_back = RCT_Less;
        return renderMode;
    }
    
    virtual bool canCastShadow() const
    {
        return false;
    }
};



PortalMeshRender::PortalMeshRender()
{
    canCastShadow = false;
    hasPrePass = true;
}

PortalMeshRender::~PortalMeshRender()
{
    delete portalMaterialInstance;
}

void PortalMeshRender::setInternalMesh()
{
    initialPlaneMesh();
    if (portalMaterialInstance == NULL) {
        portalMaterialInstance = &portalMaterial->instantiate();
        portalMaterialInstance->setStencilValue(portalID);
    }
    if (getMesh() == NULL) {
        setMesh(&planeMesh);
        setMaterial(0, *portalMaterialInstance);
    }
}

void PortalMeshRender::setPortalID(int portalID)
{
    this->portalID = portalID;
    portalMaterialInstance->setStencilValue(portalID);
}

int PortalMeshRender::getPortalID() const
{
    return portalID;
}

void PortalMeshRender::getMeshTransformData(MeshTransformData* data) const
{
    MeshRender::getMeshTransformData(data);
    data->setStencilValue(portalID);
}

void PortalMeshRender::render(RenderInfo& info)
{
    if (portalID < 0)
        return;
    MeshMaterialCollection::DispatchData dispatchData;
	dispatchData.init<PortalMeshRenderCommand>();
    dispatchData.hidden = hidden;
    dispatchData.isStatic = isStatic;
    dispatchData.canCastShadow = canCastShadow;
    dispatchData.hasPrePass = hasPrePass;
    dispatchData.hasGeometryPass = false;

    collection.dispatchMeshDraw(dispatchData);
    outlineCollection.dispatchMeshDraw(dispatchData);
}

bool PortalMeshRender::isInited = false;
Mesh PortalMeshRender::planeMesh;
Material* PortalMeshRender::portalMaterial = NULL;

void PortalMeshRender::initialPlaneMesh()
{
    if (isInited)
        return;
    
    MeshPartDesc desc = VendorManager::getInstance().getVendor().newMeshPart(4, 6);
    MeshPart& meshPart = *new MeshPart(desc);
    assert(meshPart.isValid());
    meshPart.vertex(0) = Vector3f(0, 1, 1);
    meshPart.vertex(1) = Vector3f(0, 1, -1);
    meshPart.vertex(2) = Vector3f(0, -1, -1);
    meshPart.vertex(3) = Vector3f(0, -1, 1);

    meshPart.uv(0) = Vector2f(1, 1);
    meshPart.uv(1) = Vector2f(1, 0);
    meshPart.uv(2) = Vector2f(0, 0);
    meshPart.uv(3) = Vector2f(0, 1);

    meshPart.normal(0) = Vector3f(1, 0, 0);
    meshPart.normal(1) = Vector3f(1, 0, 0);
    meshPart.normal(2) = Vector3f(1, 0, 0);
    meshPart.normal(3) = Vector3f(1, 0, 0);

    unsigned int* indices = &meshPart.element(0);
    indices[0] = 0; indices[1] = 2; indices[2] = 1;
    indices[3] = 0; indices[4] = 3; indices[5] = 2;

    planeMesh.resize(3, 2, 4);
    planeMesh.setTotalMeshPart(meshPart);
    planeMesh.addMeshPart("Plane", meshPart);

    planeMesh.bound.minPoint = Vector3f(0, -1, -1);
    planeMesh.bound.maxPoint = Vector3f(0, 1, 1);

    portalMaterial = getAssetByPath<Material>("Engine/Shaders/Portal.mat");
    assert(portalMaterial);
    portalMaterial->setStencilTest(true);
    portalMaterial->setStencilCompare(RCT_Always);
    
    isInited = true;
}

PortalRender::PortalRender()
{
    cameraRenderID = cameraRenderNextID;
    cameraRenderNextID++;
}

int PortalRender::getCameraRenderID() const
{
    return cameraRenderID;
}

void PortalRender::setRenderData(CameraRenderData* renderData)
{
    this->renderData = renderData;
}

CameraRenderData* PortalRender::getRenderData()
{
    return renderData;
}

void PortalRender::render(RenderInfo& info)
{
    CameraRenderData* cameraRenderData = getRenderData();
    if (!hidden && cameraRenderData) {
        RENDER_THREAD_ENQUEUE_TASK(CameraRenderUpdate, ([cameraRenderData] (RenderThreadContext& context)
        {
            cameraRenderData->updateSurfaceBuffer(context.renderGraph);
            updateRenderDataMainThread(cameraRenderData, Time::frames());
            context.sceneRenderData->setCamera(cameraRenderData);
        }));
    }
}
